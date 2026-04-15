// dear_widgets_stroke.cpp
// GPU stroke expansion — based on "Fast GPU stroke expansion" (HPG 2024).
// Paper: https://arxiv.org/abs/2405.00127
// Reference implementation: https://github.com/linebender/gpu-stroke-expansion-paper
// CPU: Euler spiral stroke expansion (faithful port from Vello/linebender).
// GPU: Winding-number pixel shader for zero-overdraw fill.
// This file is #include'd from dear_widgets.cpp (unity build pattern).

#ifdef _DEAR_WIDGETS_STROKE_INCLUDED

namespace ImWidgets {

// ============================================================
// Fresnel-like integral for Euler spirals
// ============================================================

static void DW_IntegEuler12(float k0, float k1, float& ou, float& ov)
{
    float t1_1=k0, t1_2=0.5f*k1;
    float t2_2=t1_1*t1_1, t2_3=2.f*(t1_1*t1_2), t2_4=t1_2*t1_2;
    float t3_4=t2_2*t1_2+t2_3*t1_1, t3_6=t2_4*t1_2;
    float t4_4=t2_2*t2_2, t4_5=2.f*(t2_2*t2_3), t4_6=2.f*(t2_2*t2_4)+t2_3*t2_3;
    float t4_7=2.f*(t2_3*t2_4), t4_8=t2_4*t2_4;
    float t5_6=t4_4*t1_2+t4_5*t1_1, t5_8=t4_6*t1_2+t4_7*t1_1, t5_10=t4_8*t1_2;
    float t6_6=t4_4*t2_2, t6_7=t4_4*t2_3+t4_5*t2_2;
    float t6_8=t4_4*t2_4+t4_5*t2_3+t4_6*t2_2;
    float t6_9=t4_5*t2_4+t4_6*t2_3+t4_7*t2_2;
    float t6_10=t4_6*t2_4+t4_7*t2_3+t4_8*t2_2;
    float t7_8=t6_6*t1_2+t6_7*t1_1, t7_10=t6_8*t1_2+t6_9*t1_1;
    float t8_8=t6_6*t2_2, t8_9=t6_6*t2_3+t6_7*t2_2;
    float t8_10=t6_6*t2_4+t6_7*t2_3+t6_8*t2_2;
    float t9_10=t8_8*t1_2+t8_9*t1_1, t10_10=t8_8*t2_2;
    float u=1.f;
    u-=(1.f/24)*t2_2+(1.f/160)*t2_4;
    u+=(1.f/1920)*t4_4+(1.f/10752)*t4_6+(1.f/55296)*t4_8;
    u-=(1.f/322560)*t6_6+(1.f/1658880)*t6_8+(1.f/8110080)*t6_10;
    u+=(1.f/92897280)*t8_8+(1.f/454164480)*t8_10;
    u-=2.4464949595157930e-11f*t10_10;
    float v=(1.f/12)*t1_2;
    v-=(1.f/480)*t3_4+(1.f/2688)*t3_6;
    v+=(1.f/53760)*t5_6+(1.f/276480)*t5_8+(1.f/1351680)*t5_10;
    v-=(1.f/11612160)*t7_8+(1.f/56770560)*t7_10;
    v+=2.4464949595157932e-10f*t9_10;
    ou=u; ov=v;
}
static void DW_IntegEuler12N(float k0, float k1, int n, float& ou, float& ov)
{
    float th1=k0, th2=0.5f*k1, ds=1.f/(float)n;
    k0*=ds; k1*=ds;
    float x=0, y=0, s0=0.5f*ds-0.5f;
    for(int i=0;i<n;++i){
        float s=s0+ds*(float)i, km0=k1*s+k0, km1=k1*ds;
        float u,v; DW_IntegEuler12(km0,km1,u,v);
        float th=(th2*s+th1)*s, ct=cosf(th), st=sinf(th);
        x+=ct*u-st*v; y+=ct*v+st*u;
    }
    ou=x*ds; ov=y*ds;
}
static void DW_IntegEuler(float k0, float k1, float& ou, float& ov)
{
    float c1=ImFabs(k1), c0=ImFabs(k0)+0.5f*c1;
    float e=0.006f*c0*c0+0.029f*c1, e3=e*e*e, e6=e3*e3;
    if(e6<1e-6f){DW_IntegEuler12(k0,k1,ou,ov);return;}
    int n=(int)ceilf(e/powf(1e-6f,1.f/6.f)); if(n<2)n=2;
    DW_IntegEuler12N(k0,k1,n,ou,ov);
}

// ============================================================
// EulerParams + EulerSeg
// ============================================================

struct DW_EulerParams {
    float th0,th1,k0,k1,ch;
    static DW_EulerParams FromAngles(float th0, float th1){
        float k1_old=0, dth=th1-th0, k0=th0+th1;
        float k1=(6.f-(1.f/70)*dth*dth-0.1f*k0*k0)*dth, eo=dth;
        for(int i=0;i<10;++i){
            float u,v; DW_IntegEuler(k0,k1,u,v);
            float er=dth-(0.25f*k1-2.f*atan2f(v,u));
            if(ImFabs(er)<1e-6f){ DW_EulerParams p={th0,th1,k0,k1,sqrtf(u*u+v*v)}; return p; }
            float d=er-eo; if(ImFabs(d)<1e-12f)break;
            float nk=k1+(k1_old-k1)*er/d; k1_old=k1; eo=er; k1=nk;
        }
        float u,v; DW_IntegEuler(k0,k1,u,v);
        DW_EulerParams p={th0,th1,k0,k1,sqrtf(u*u+v*v)}; return p;
    }
    float EvalTh(float t) const { return (k0+0.5f*k1*(t-1))*t-th0; }
    ImVec2 Eval(float t) const {
        float thm=EvalTh(t*0.5f); float u,v;
        DW_IntegEuler((k0+k1*(0.5f*t-0.5f))*t, k1*t*t, u, v);
        float s=t/ch*sinf(thm), c=t/ch*cosf(thm);
        return ImVec2(u*c-v*s, -v*c-u*s);
    }
    ImVec2 EvalWithOffset(float t, float offset) const {
        float th=EvalTh(t); ImVec2 p=Eval(t);
        return ImVec2(p.x+offset*sinf(th), p.y+offset*cosf(th));
    }
    // Curvature scaled to unit chord. Divide by chord_len for world curvature.
    float Curvature(float t) const { return (k0+k1*(t-0.5f))*ch; }
    // Evolute = offset by -1/curvature (center of curvature)
    ImVec2 EvalEvolute(float t) const {
        float k = Curvature(t);
        if (ImFabs(k) < 1e-9f) return Eval(t); // degenerate: straight line
        return EvalWithOffset(t, -1.0f / k);
    }
};

struct DW_EulerSeg {
    ImVec2 p0,p1; DW_EulerParams params;
    float ChordLen() const { return sqrtf((p1.x-p0.x)*(p1.x-p0.x)+(p1.y-p0.y)*(p1.y-p0.y)); }
    ImVec2 EvalWithOffset(float t, float normalized_offset) const {
        ImVec2 c(p1.x-p0.x, p1.y-p0.y);
        ImVec2 n=params.EvalWithOffset(t, normalized_offset);
        return ImVec2(p0.x+c.x*n.x-c.y*n.y, p0.y+c.x*n.y+c.y*n.x);
    }
    // Evolute point in screen space
    ImVec2 EvalEvolute(float t) const {
        ImVec2 c(p1.x-p0.x, p1.y-p0.y);
        ImVec2 n=params.EvalEvolute(t);
        return ImVec2(p0.x+c.x*n.x-c.y*n.y, p0.y+c.x*n.y+c.y*n.x);
    }
    ImVec2 EvalTangent(float t) const {
        float th=params.EvalTh(t);
        ImVec2 c(p1.x-p0.x,p1.y-p0.y); float cl=sqrtf(c.x*c.x+c.y*c.y);
        if(cl<1e-6f)return ImVec2(1,0);
        float ca=cosf(th),sa=sinf(th); ImVec2 d(c.x/cl,c.y/cl);
        return ImVec2(d.x*ca-d.y*sa, d.x*sa+d.y*ca);
    }
};

// ============================================================
// ESPC integral approximation — from flatten.rs
// ============================================================

static float DW_EspcIntApprox(float x){
    float y=ImFabs(x), a;
    if(y<0.8f) a=sinf(1.0976991822760038f*y)*(1.f/1.0976991822760038f);
    else if(y<1.25f){ float d=y-1; a=(sqrtf(8.f)/3)*d*sqrtf(ImFabs(d))+IM_PI*0.25f; }
    else{ float qa,qb,qc;
        if(y<2.1f){qa=0.6406f;qb=-0.81f;qc=0.9148117935952064f;}
        else{qa=0.5f;qb=-0.156f;qc=0.16145779359520596f;}
        a=qa*y*y+qb*y+qc; }
    return x>=0?a:-a;
}
static float DW_EspcIntInvApprox(float x){
    float y=ImFabs(x), a;
    if(y<0.7010707591262915f)
        a=asinf(ImClamp(y*1.0976991822760038f, 0.f, 1.f))*(1.f/1.0976991822760038f);
    else if(y<0.903249293595206f){
        float b=y-IM_PI*0.25f, u_abs=powf(ImFabs(b),2.f/3);
        a=(b>=0?u_abs:-u_abs)*cbrtf(9.f/8)+1.f;
    } else {
        float u,v,w;
        if(y<2.038857793595206f){float B=0.5f*(-0.81f)/0.6406f;u=B*B-0.9148117935952064f/0.6406f;v=1.f/0.6406f;w=B;}
        else{float B=0.5f*(-0.156f)/0.5f;u=B*B-0.16145779359520596f/0.5f;v=1.f/0.5f;w=B;}
        a=sqrtf(ImMax(0.f,u+v*y))-w;
    }
    return x>=0?a:-a;
}

// ============================================================
// CubicParams — error estimation
// ============================================================

struct DW_CubicParams {
    float th0,th1,d0,d1,err,chord_len;
    static DW_CubicParams FromPointsDerivs(ImVec2 p0, ImVec2 p1, ImVec2 q0, ImVec2 q1, float dt){
        ImVec2 ch(p1.x-p0.x,p1.y-p0.y);
        float ch2=ch.x*ch.x+ch.y*ch.y, sc=(ch2>1e-12f)?dt/ch2:0;
        float h0x=q0.x*ch.x+q0.y*ch.y, h0y=q0.y*ch.x-q0.x*ch.y;
        float h1x=q1.x*ch.x+q1.y*ch.y, h1y=q1.x*ch.y-q1.y*ch.x;
        DW_CubicParams cp;
        cp.th0=atan2f(h0y,h0x); cp.d0=sqrtf(h0x*h0x+h0y*h0y)*sc;
        cp.th1=atan2f(h1y,h1x); cp.d1=sqrtf(h1x*h1x+h1y*h1y)*sc;
        cp.chord_len=sqrtf(ch2);
        // est_euler_err
        float ct0=cosf(cp.th0),ct1=cosf(cp.th1);
        float e0=(2.f/3)/ImMax(1e-6f,1+ct0), e1=(2.f/3)/ImMax(1e-6f,1+ct1);
        float s0=sinf(cp.th0),s1=sinf(cp.th1),s01=sinf(s0+s1);
        float amin=0.15f*(2*e0*s0+2*e1*s1-e0*e1*s01);
        float a=0.15f*(2*cp.d0*s0+2*cp.d1*s1-cp.d0*cp.d1*s01);
        float aerr=ImFabs(a-amin);
        float sym=ImFabs(cp.th0+cp.th1), asym=ImFabs(cp.th0-cp.th1);
        float dist=sqrtf((cp.d0-e0)*(cp.d0-e0)+(cp.d1-e1)*(cp.d1-e1));
        float s2=sym*sym;
        cp.err=1.25f*(3.7e-6f*s2*s2*sym+6e-3f*asym*s2)+1.55f*aerr+5e-3f*sym*dist+7e-2f*asym*dist;
        return cp;
    }
};

// ============================================================
// Cubic eval + Cubic-to-Euler conversion
// ============================================================

static void DW_CubicEvalAndDeriv(ImVec2 c0,ImVec2 c1,ImVec2 c2,ImVec2 c3,
    float t, ImVec2& pos, ImVec2& der){
    float u=1-t,uu=u*u,ut=u*t,tt=t*t;
    pos.x=c0.x*(uu*u)+c1.x*(3*uu*t)+c2.x*(3*u*tt)+c3.x*(tt*t);
    pos.y=c0.y*(uu*u)+c1.y*(3*uu*t)+c2.y*(3*u*tt)+c3.y*(tt*t);
    der.x=(c1.x-c0.x)*uu+(c2.x-c1.x)*(2*ut)+(c3.x-c2.x)*tt;
    der.y=(c1.y-c0.y)*uu+(c2.y-c1.y)*(2*ut)+(c3.y-c2.y)*tt;
}

// De Casteljau subdivision: split cubic at parameter t into two cubics.
static void DW_CubicSubdivide(ImVec2 p0, ImVec2 p1, ImVec2 p2, ImVec2 p3, float t,
    ImVec2 left[4], ImVec2 right[4])
{
    ImVec2 m01(p0.x+(p1.x-p0.x)*t, p0.y+(p1.y-p0.y)*t);
    ImVec2 m12(p1.x+(p2.x-p1.x)*t, p1.y+(p2.y-p1.y)*t);
    ImVec2 m23(p2.x+(p3.x-p2.x)*t, p2.y+(p3.y-p2.y)*t);
    ImVec2 m012(m01.x+(m12.x-m01.x)*t, m01.y+(m12.y-m01.y)*t);
    ImVec2 m123(m12.x+(m23.x-m12.x)*t, m12.y+(m23.y-m12.y)*t);
    ImVec2 mid(m012.x+(m123.x-m012.x)*t, m012.y+(m123.y-m012.y)*t);
    left[0]=p0; left[1]=m01; left[2]=m012; left[3]=mid;
    right[0]=mid; right[1]=m123; right[2]=m23; right[3]=p3;
}

static void DW_CubicToEulerSegs(ImVec2 c0,ImVec2 c1,ImVec2 c2,ImVec2 c3,
    float tolerance, ImVector<DW_EulerSeg>& out)
{
    ImVec2 lp=c0, lq(c1.x-c0.x,c1.y-c0.y);
    if(lq.x*lq.x+lq.y*lq.y<1e-12f){ImVec2 tmp; DW_CubicEvalAndDeriv(c0,c1,c2,c3,1e-6f,tmp,lq);}
    float lt=0; ImU64 t0_u=0; float dt=1;
    for(int iter=0;iter<10000;++iter){
        float t0=(float)t0_u*dt; if(t0>=1)break;
        float t1=t0+dt; if(t1>1)t1=1;
        ImVec2 p1,q1; DW_CubicEvalAndDeriv(c0,c1,c2,c3,t1,p1,q1);
        if(q1.x*q1.x+q1.y*q1.y<1e-12f){
            ImVec2 pb,qb; DW_CubicEvalAndDeriv(c0,c1,c2,c3,t1-1e-6f,pb,qb);
            q1=qb; if(t1<1){p1=pb;t1-=1e-6f;}
        }
        DW_CubicParams cp=DW_CubicParams::FromPointsDerivs(lp,p1,lq,q1,t1-lt);
        if(cp.err*cp.chord_len<=tolerance || dt<1e-6f){
            DW_EulerParams ep=DW_EulerParams::FromAngles(cp.th0,cp.th1);
            DW_EulerSeg seg; seg.p0=lp; seg.p1=p1; seg.params=ep;
            out.push_back(seg);
            lp=p1; lq=q1; lt=t1;
            t0_u+=1;
            if(t0_u>0){unsigned sh=0;ImU64 tmp=t0_u;while((tmp&1)==0){sh++;tmp>>=1;}t0_u>>=sh;dt*=(float)(1ULL<<sh);}
        } else { t0_u*=2; dt*=0.5f; }
    }
}

// ============================================================
// flatten_euler — port of Vello's flatten_euler
// Appends offset curve points to out[] (not including start point).
// ============================================================

enum DW_EspcRobust { DW_Espc_Normal, DW_Espc_LowK1, DW_Espc_LowDist };

// ============================================================
// Range-aware flatten_offset — port of reference flatten_offset(es, range, offset, tol)
// Flattens sub-range [range_start, range_end] of an Euler spiral at given offset.
// ============================================================

static void DW_FlattenEulerRange(const DW_EulerSeg& es, float range_start, float range_end,
    float offset, float tol, ImVector<ImVec2>& out)
{
    float chord_len = es.ChordLen();
    if (chord_len < 1e-6f) { out.push_back(es.EvalWithOffset(range_end, offset / chord_len)); return; }

    float range_size = range_end - range_start;
    float k0 = es.params.k0 + (range_start - 0.5f) * es.params.k1;
    float k1 = es.params.k1 * range_size;
    float dist_scaled = (offset / chord_len) * es.params.ch;
    float scale_mul = 0.5f * 0.7071067811865476f * sqrtf(chord_len / (es.params.ch * tol));

    float a=0,b=0,integral=0,int0=0,n_frac;
    DW_EspcRobust robust;

    if (ImFabs(k1) < 1e-3f) {
        float k = k0 + 0.5f * k1;
        n_frac = sqrtf(ImFabs(k * (k * dist_scaled + 1)));
        robust = DW_Espc_LowK1;
    } else if (ImFabs(dist_scaled) < 1e-3f) {
        a = k1; b = k0;
        int0 = b * sqrtf(ImFabs(b));
        float ab = a + b; integral = ab * sqrtf(ImFabs(ab)) - int0;
        n_frac = (ImFabs(a) > 1e-9f) ? (2.f/3) * integral / a : 0;
        robust = DW_Espc_LowDist;
    } else {
        a = -2 * dist_scaled * k1;
        b = -1 - 2 * dist_scaled * k0;
        int0 = DW_EspcIntApprox(b);
        integral = DW_EspcIntApprox(a + b) - int0;
        float k_peak = k0 - k1 * b / a;
        float ip = sqrtf(ImFabs(k_peak * (k_peak * dist_scaled + 1)));
        n_frac = (ImFabs(a) > 1e-9f) ? integral * ip / a : 0;
        robust = DW_Espc_Normal;
    }

    float n_f = ceilf(ImClamp(ImFabs(n_frac) * scale_mul * range_size, 1.f, 100.f));
    int n = (int)n_f;

    for (int i = 0; i < n; ++i) {
        ImVec2 lp1;
        if (i == n - 1) {
            lp1 = es.EvalWithOffset(range_end, offset / chord_len);
        } else {
            float t = (float)(i + 1) / n_f;
            float s;
            switch (robust) {
            case DW_Espc_LowK1: s = t; break;
            case DW_Espc_LowDist: {
                float c = cbrtf(integral * t + int0);
                s = (ImFabs(a) > 1e-9f) ? (c * ImFabs(c) - b) / a : t;
            } break;
            default: {
                float inv = DW_EspcIntInvApprox(integral * t + int0);
                s = (ImFabs(a) > 1e-9f) ? (inv - b) / a : t;
            } break;
            }
            s = ImClamp(s, 0.f, 1.f);
            lp1 = es.EvalWithOffset(range_start + range_size * s, offset / chord_len);
        }
        out.push_back(lp1);
    }
}

// ============================================================
// lower_es_evolute (Line version) — port of default Lowering::lower_es_evolute
// Generates evolute curve points for the given range of an Euler segment.
// ============================================================

static void DW_LowerEsEvolute(const DW_EulerSeg& es, float range_start, float range_end,
    float tol, ImVector<ImVec2>& out)
{
    float range_size = range_end - range_start;
    float arc_len = es.ChordLen() / es.params.ch;
    float k0 = es.params.k0, k1 = es.params.k1;
    if (ImFabs(k1) < 1e-9f) { out.push_back(es.EvalEvolute(range_end)); return; }

    float ratio = k0 / k1;
    float rho_int_0 = sqrtf(ImFabs(0.5f * (ratio + range_start - 0.5f)));
    float rho_int_1 = sqrtf(ImFabs(0.5f * (ratio + range_end - 0.5f)));
    float rho_int = rho_int_1 - rho_int_0;
    float n_subdiv = ceilf(ImMax(1.f, range_size * ImFabs(rho_int) * sqrtf(arc_len / tol)));
    int n = (int)n_subdiv;
    float sign2 = (ratio >= 0) ? 2.f : -2.f;

    for (int i = 1; i <= n; ++i) {
        float t = (float)i / n_subdiv;
        float u = rho_int_0 + t * rho_int;
        float s = range_start + range_size * (sign2 * u * u + 0.5f - ratio);
        s = ImClamp(s, range_start, range_end);
        out.push_back(es.EvalEvolute(s));
    }
}

// ============================================================
// ArcSegment lower_arc — exact port of reference Line::lower_arc
// ArcSegment: p0, p1 are endpoints, k0 is signed arc angle.
// ============================================================

static void DW_LowerArc(ImVec2 arc_p0, ImVec2 arc_p1, float k0, float tol,
    ImVector<ImVec2>& out)
{
    ImVec2 chord(arc_p1.x - arc_p0.x, arc_p1.y - arc_p0.y);
    float chord_len = sqrtf(chord.x*chord.x + chord.y*chord.y);
    float n_frac = ImFabs(k0) * sqrtf(0.125f * chord_len / tol);
    if (n_frac <= 1.f) {
        out.push_back(arc_p1);
        return;
    }
    float n_ceil = ceilf(n_frac);
    int n = (int)n_ceil;
    float half_k = 0.5f * k0;
    float s = sinf(half_k);
    float c = cosf(half_k);
    float scale = (ImFabs(s) > 1e-9f) ? 0.5f / s : 0.f;
    ImVec2 p0 = arc_p0;
    for (int i = 1; i < n; ++i) {
        float th = ((float)i / n_ceil - 0.5f) * k0;
        float u = sinf(th) * scale + 0.5f;
        float v = (cosf(th) - c) * scale;
        ImVec2 p1(arc_p0.x + u*chord.x - v*chord.y,
                   arc_p0.y + u*chord.y + v*chord.x);
        out.push_back(p1);
        p0 = p1;
    }
    out.push_back(arc_p1);
}

// round_join: exact port of reference round_join.
// Appends arc from the current last point of 'out' to (center - norm),
// with arc angle = -angle.
static void DW_RoundJoin(ImVector<ImVec2>& out, float tol,
    ImVec2 center, ImVec2 norm, float angle)
{
    ImVec2 p1(center.x - norm.x, center.y - norm.y);
    ImVec2 last = out[out.Size - 1];
    DW_LowerArc(last, p1, -angle, tol, out);
}

// round_cap = round_join with angle=PI
static void DW_RoundCap(ImVector<ImVec2>& out, float tol,
    ImVec2 center, ImVec2 norm)
{
    DW_RoundJoin(out, tol, center, norm, IM_PI);
}

// ============================================================
// Stroke expansion — builds closed polygon, renders via AddConcavePolyFilled
// Matches Vello production code structure: flatten_euler(+offset),
// flatten_euler(-offset), draw_join, draw_cap per segment.
// ============================================================

static bool s_strokeDebugWireframe = false;

void SetStrokeDebugWireframe(bool v) { s_strokeDebugWireframe = v; }
bool GetStrokeDebugWireframe()       { return s_strokeDebugWireframe; }

// ============================================================
// StrokeContour — cusp-aware offset path accumulator.
// Port of reference StrokeContour<Line>.
// ============================================================

struct DW_StrokeContour {
    ImVector<ImVec2> points;      // main path points
    ImVector<ImVec2> rev_points;  // reversed parallel (past cusp)
    ImVector<ImVec2> evo_points;  // evolute (past cusp)
    bool has_cusp;

    DW_StrokeContour() : has_cusp(false) {}

    void Start() {
        if (!has_cusp) {
            has_cusp = true;
            evo_points.resize(0);
            rev_points.resize(0);
            if (points.Size > 0) {
                evo_points.push_back(points[points.Size - 1]);
                rev_points.push_back(points[points.Size - 1]);
            }
        }
    }

    void Finalize() {
        if (!has_cusp) return;
        // Stitch: evolute → rev_parallel_reversed → evolute
        if (evo_points.Size > 0 && rev_points.Size > 0) {
            // Connect evolute end to rev_parallel end
            evo_points.push_back(rev_points[rev_points.Size - 1]);
            // Append evolute
            for (int i = 1; i < evo_points.Size; ++i)
                points.push_back(evo_points[i]);
            // Append rev_parallel reversed
            for (int i = rev_points.Size - 2; i >= 0; --i)
                points.push_back(rev_points[i]);
            // Append evolute again
            for (int i = 1; i < evo_points.Size; ++i)
                points.push_back(evo_points[i]);
        }
        evo_points.resize(0);
        rev_points.resize(0);
        has_cusp = false;
    }

    // Process one Euler segment with cusp detection.
    // h = signed offset in pixels (negative for fwd, positive for bwd).
    // strong = true for correct evolute geometry, false for weak mode.
    void DoEulerSeg(const DW_EulerSeg& es, float h, float tol, bool strong) {
        float chord_len = es.ChordLen();
        if (chord_len < 1e-6f) return;

        float cusp0 = es.params.Curvature(0) * h + chord_len;
        float cusp1 = es.params.Curvature(1) * h + chord_len;
        float t = (cusp0 * cusp1 >= 0) ? 1.0f : cusp0 / (cusp0 - cusp1);

        if (cusp0 >= 0) {
            Finalize();
            DW_FlattenEulerRange(es, 0, t, h, tol, points);
            if (t < 1.0f) {
                if (strong) {
                    Start();
                    DW_LowerEsEvolute(es, t, 1.0f, tol, evo_points);
                    DW_FlattenEulerRange(es, t, 1.0f, h, tol, rev_points);
                } else {
                    DW_FlattenEulerRange(es, t, 1.0f, h, tol, points);
                }
            }
        } else {
            if (strong) {
                Start();
                evo_points.push_back(es.EvalEvolute(0));
                DW_LowerEsEvolute(es, 0, t, tol, evo_points);
                DW_FlattenEulerRange(es, 0, t, h, tol, rev_points);
            } else {
                DW_FlattenEulerRange(es, 0, t, h, tol, points);
            }
            if (t < 1.0f) {
                Finalize();
                DW_FlattenEulerRange(es, t, 1.0f, h, tol, points);
            }
        }
    }
};

// Flatten one cubic Bezier's offset curves (both sides) with cusp handling.
// Internally subdivides into Euler spiral segments.
static void DW_FlattenCubicOffset(ImVec2 c0, ImVec2 c1, ImVec2 c2, ImVec2 c3,
    float half_w, float tol,
    ImVec2 fwd_start, ImVec2 fwd_end,
    ImVec2 bwd_start, ImVec2 bwd_end,
    DW_StrokeContour& fwd_contour, DW_StrokeContour& bwd_contour,
    bool strong)
{
    IM_UNUSED(fwd_start); IM_UNUSED(fwd_end);
    IM_UNUSED(bwd_start); IM_UNUSED(bwd_end);
    ImVector<DW_EulerSeg> segs;
    DW_CubicToEulerSegs(c0, c1, c2, c3, tol, segs);
    if (segs.Size == 0) return;

    for (int i = 0; i < segs.Size; ++i) {
        const DW_EulerSeg& es = segs[i];
        float cl = es.ChordLen();
        if (cl < 1e-6f) continue;

        // fwd = -half_w offset, bwd = +half_w offset
        fwd_contour.DoEulerSeg(es, -half_w, tol, strong);
        bwd_contour.DoEulerSeg(es,  half_w, tol, strong);
    }

    // Snap first/last points to caller-provided endpoints for continuity
    if (fwd_contour.points.Size > 0) {
        // Replace first point added by this cubic with caller's start
        // (only if this is the first segment overall — handled by caller)
    }
}

// Exact port of reference do_join.
// Convention: fwd = p - norm ("forward" in reference), bwd = p + norm ("backward").
// tan0 = incoming tangent direction (not normalized), norm = half_w-scaled normal.
// join_thresh = 2*tol/width.
static void DW_DoJoin(ImVec2 p0, ImVec2 last_tan, ImVec2 tan0, ImVec2 norm,
    ImWidgetsJoin join_style, float miter_limit, float half_w, float tol,
    float join_thresh,
    ImVector<ImVec2>& fwd, ImVector<ImVec2>& bwd)
{
    ImVec2 ab = last_tan, cd = tan0;
    float cross = ab.x*cd.y - ab.y*cd.x;
    float dot   = ab.x*cd.x + ab.y*cd.y;
    float hypot = sqrtf(cross*cross + dot*dot);

    if (dot <= 0.f || ImFabs(cross) >= hypot * join_thresh) {
        if (join_style == ImWidgetsJoin_Bevel) {
            fwd.push_back(ImVec2(p0.x - norm.x, p0.y - norm.y));
            bwd.push_back(ImVec2(p0.x + norm.x, p0.y + norm.y));
        } else if (join_style == ImWidgetsJoin_Mitter) {
            // Vello PR #1323: guard against near-collinear tangents producing
            // degenerate miter geometry. Fall back to bevel when cross is tiny.
            const float TANGENT_THRESH = 1e-6f;
            if (2.f * hypot < (hypot + dot) * miter_limit * miter_limit
                && ImFabs(cross) > TANGENT_THRESH * TANGENT_THRESH) {
                float last_scale = half_w / sqrtf(ab.x*ab.x + ab.y*ab.y + 1e-12f);
                ImVec2 last_norm(-ab.y * last_scale, ab.x * last_scale);
                if (cross > 0.f) {
                    ImVec2 fp_last(p0.x - last_norm.x, p0.y - last_norm.y);
                    ImVec2 fp_this(p0.x - norm.x, p0.y - norm.y);
                    float h = (ab.x*(fp_this.y-fp_last.y) - ab.y*(fp_this.x-fp_last.x)) / cross;
                    ImVec2 miter_pt(fp_this.x - cd.x*h, fp_this.y - cd.y*h);
                    fwd.push_back(miter_pt);
                } else if (cross < 0.f) {
                    ImVec2 bp_last(p0.x + last_norm.x, p0.y + last_norm.y);
                    ImVec2 bp_this(p0.x + norm.x, p0.y + norm.y);
                    float h = (ab.x*(bp_this.y-bp_last.y) - ab.y*(bp_this.x-bp_last.x)) / cross;
                    ImVec2 miter_pt(bp_this.x - cd.x*h, bp_this.y - cd.y*h);
                    bwd.push_back(miter_pt);
                }
            }
            fwd.push_back(ImVec2(p0.x - norm.x, p0.y - norm.y));
            bwd.push_back(ImVec2(p0.x + norm.x, p0.y + norm.y));
        } else { // Round
            float angle = atan2f(cross, dot);
            if (angle > 0.f) {
                bwd.push_back(ImVec2(p0.x + norm.x, p0.y + norm.y));
                DW_RoundJoin(fwd, tol, p0, norm, angle);
            } else {
                fwd.push_back(ImVec2(p0.x - norm.x, p0.y - norm.y));
                ImVec2 neg_norm(-norm.x, -norm.y);
                DW_RoundJoin(bwd, tol, p0, neg_norm, angle);
            }
        }
    }
}

// Build stroke outline for a cubic Bezier path.
// Convention: fwd = p - norm (reference "forward"), bwd = p + norm (reference "backward").
// Offset curves: fwd gets -half_w offset, bwd gets +half_w offset.
static void DW_StrokeBuildOutlineCubics(
    const ImVec2* cubics, int n_cubics,
    float half_w, float tol,
    ImWidgetsCap cap_style, ImWidgetsJoin join_style, float miter_limit,
    bool closed, bool strong,
    ImVector<ImVec2>& fwd, ImVector<ImVec2>& bwd)
{
    IM_UNUSED(cap_style);
    float join_thresh = 2.f * tol / (2.f * half_w);
    ImVec2 last_tan(0,0);
    bool started = false;
    DW_StrokeContour fwd_contour, bwd_contour;

    for (int ci = 0; ci < n_cubics; ++ci) {
        ImVec2 c0=cubics[ci*3], c1=cubics[ci*3+1], c2=cubics[ci*3+2], c3=cubics[ci*3+3];
        ImVec2 chord(c3.x-c0.x, c3.y-c0.y);
        float cl=sqrtf(chord.x*chord.x+chord.y*chord.y);
        if(cl<1e-6f) continue;

        // Start/end tangents
        ImVec2 tan_s(c1.x-c0.x, c1.y-c0.y);
        if(tan_s.x*tan_s.x+tan_s.y*tan_s.y<1e-12f) tan_s=chord;
        ImVec2 tan_e(c3.x-c2.x, c3.y-c2.y);
        if(tan_e.x*tan_e.x+tan_e.y*tan_e.y<1e-12f) tan_e=chord;
        float tsl=sqrtf(tan_s.x*tan_s.x+tan_s.y*tan_s.y);
        float tel=sqrtf(tan_e.x*tan_e.x+tan_e.y*tan_e.y);
        ImVec2 n_s(-tan_s.y/tsl*half_w, tan_s.x/tsl*half_w);
        ImVec2 n_e(-tan_e.y/tel*half_w, tan_e.x/tel*half_w);

        // fwd = p-n, bwd = p+n (reference convention)
        ImVec2 fwd_s(c0.x-n_s.x, c0.y-n_s.y), fwd_e(c3.x-n_e.x, c3.y-n_e.y);
        ImVec2 bwd_s(c0.x+n_s.x, c0.y+n_s.y), bwd_e(c3.x+n_e.x, c3.y+n_e.y);

        if(!started) {
            started = true;
            fwd.push_back(fwd_s);
            bwd.push_back(bwd_s);
        } else {
            DW_DoJoin(c0, last_tan, tan_s, n_s, join_style, miter_limit, half_w, tol, join_thresh, fwd, bwd);
        }

        // Flatten offset curves with cusp handling
        DW_FlattenCubicOffset(c0,c1,c2,c3, half_w, tol, fwd_s, fwd_e, bwd_s, bwd_e,
            fwd_contour, bwd_contour, strong);

        // Finalize any pending cusp regions and transfer points
        fwd_contour.Finalize();
        bwd_contour.Finalize();
        for (int i = 0; i < fwd_contour.points.Size; ++i) fwd.push_back(fwd_contour.points[i]);
        for (int i = 0; i < bwd_contour.points.Size; ++i) bwd.push_back(bwd_contour.points[i]);
        fwd_contour.points.resize(0);
        bwd_contour.points.resize(0);

        last_tan = tan_e;
    }

    if(closed && started) {
        ImVec2 last_pt = cubics[n_cubics * 3];
        ImVec2 first_pt = cubics[0];
        ImVec2 seg(first_pt.x - last_pt.x, first_pt.y - last_pt.y);
        float seg_len = sqrtf(seg.x*seg.x + seg.y*seg.y);
        if (seg_len > 1e-6f) {
            float sc = half_w / seg_len;
            ImVec2 norm(-seg.y*sc, seg.x*sc);
            DW_DoJoin(last_pt, last_tan, seg, norm, join_style, miter_limit, half_w, tol, join_thresh, fwd, bwd);
            fwd.push_back(ImVec2(first_pt.x - norm.x, first_pt.y - norm.y));
            bwd.push_back(ImVec2(first_pt.x + norm.x, first_pt.y + norm.y));
            last_tan = seg;
        }
        ImVec2 c0=cubics[0], c1=cubics[1];
        ImVec2 tan_s(c1.x-c0.x, c1.y-c0.y);
        if(tan_s.x*tan_s.x+tan_s.y*tan_s.y<1e-12f){ImVec2 c3=cubics[3];tan_s=ImVec2(c3.x-c0.x,c3.y-c0.y);}
        float tsl=sqrtf(tan_s.x*tan_s.x+tan_s.y*tan_s.y);
        ImVec2 n_s(-tan_s.y/tsl*half_w, tan_s.x/tsl*half_w);
        DW_DoJoin(cubics[0], last_tan, tan_s, n_s, join_style, miter_limit, half_w, tol, join_thresh, fwd, bwd);
    }
}

// Build stroke outline for a polyline.
// Same convention: fwd = p - norm, bwd = p + norm.
static void DW_StrokeBuildOutlinePolyline(
    const ImVec2* points, int n_points,
    float half_w, float tol,
    ImWidgetsCap cap_style, ImWidgetsJoin join_style, float miter_limit,
    bool closed,
    ImVector<ImVec2>& fwd, ImVector<ImVec2>& bwd)
{
    IM_UNUSED(cap_style);
    float join_thresh = 2.f * tol / (2.f * half_w);
    ImVec2 last_tan(0,0);
    bool started = false;
    int n_segs = closed ? n_points : n_points - 1;

    for(int i=0; i<n_segs; ++i){
        int i0=i, i1=(i+1)%n_points;
        ImVec2 tangent(points[i1].x-points[i0].x, points[i1].y-points[i0].y);
        float tl=sqrtf(tangent.x*tangent.x+tangent.y*tangent.y);
        if(tl<1e-6f) continue;
        float sc = half_w / tl;
        ImVec2 norm(-tangent.y*sc, tangent.x*sc);

        if(!started){
            started = true;
            fwd.push_back(ImVec2(points[i0].x-norm.x, points[i0].y-norm.y));
            bwd.push_back(ImVec2(points[i0].x+norm.x, points[i0].y+norm.y));
        } else {
            DW_DoJoin(points[i0], last_tan, tangent, norm, join_style, miter_limit, half_w, tol, join_thresh, fwd, bwd);
        }

        // Line endpoint
        fwd.push_back(ImVec2(points[i1].x-norm.x, points[i1].y-norm.y));
        bwd.push_back(ImVec2(points[i1].x+norm.x, points[i1].y+norm.y));
        last_tan = tangent;
    }

    if(closed && started){
        int i1=1%n_points;
        ImVec2 tangent(points[i1].x-points[0].x, points[i1].y-points[0].y);
        float tl=sqrtf(tangent.x*tangent.x+tangent.y*tangent.y);
        if(tl>1e-6f){
            float sc=half_w/tl;
            ImVec2 norm(-tangent.y*sc, tangent.x*sc);
            DW_DoJoin(points[0], last_tan, tangent, norm, join_style, miter_limit, half_w, tol, join_thresh, fwd, bwd);
        }
    }
}

// Callback: upload stroke uniform buffer before shader activation.
struct DW_StrokeCBData {
    ImPlatform_ShaderProgram program;
    ImWidgetsStrokeBuffer* buf;
    int buf_size;
};

static ImVector<DW_StrokeCBData*> s_strokeCBPool;
static int s_strokeCBPoolIdx = 0;
static ImU64 s_strokeCBFrame = 0;

static void DW_PrepareStrokeUniforms(const ImDrawList*, const ImDrawCmd* cmd)
{
    DW_StrokeCBData* data = (DW_StrokeCBData*)cmd->UserCallbackData;
    if (!data || !data->program) return;
    ImPlatform_SetShaderUniform(data->program, "strokeBuffer", data->buf, data->buf_size);
}

// Build line soup from fwd/bwd/cap arrays and render via winding-number shader.
// Zero overdraw guaranteed — each pixel evaluated exactly once.
static void DW_StrokeRenderOutline(ImDrawList* dl,
    ImVector<ImVec2>& fwd, ImVector<ImVec2>& bwd,
    ImU32 col, float half_w, ImWidgetsCap cap, bool closed,
    float tol, ImVec2 start_pt, ImVec2 end_pt, ImVec2 start_norm)
{
    if (fwd.Size < 2 || bwd.Size < 2 || (col & IM_COL32_A_MASK) == 0) return;

    // --- Lazy-init stroke shader ---
    if (gs_pContext->strokeShader.program == NULL)
        CreateInternalShader(&gs_pContext->strokeShader, "stroke", 0, NULL, 0, NULL);
    ImPlatform_ShaderProgram program = gs_pContext->strokeShader.program;
    if (!program) return; // shader compile failed

    // --- Build cap point arrays (same as reference finish()) ---
    ImVector<ImVec2> end_cap_pts, start_cap_pts;
    if (!closed) {
        ImVec2 return_p = bwd[bwd.Size - 1];
        ImVec2 d(end_pt.x - return_p.x, end_pt.y - return_p.y);
        // End cap
        if (cap == ImWidgetsCap_Round) {
            end_cap_pts.push_back(fwd[fwd.Size - 1]);
            DW_RoundCap(end_cap_pts, tol, end_pt, d);
        } else if (cap == ImWidgetsCap_Square) {
            ImVec2 nx(d.x, d.y), ny(-d.y, d.x);
            end_cap_pts.push_back(fwd[fwd.Size - 1]);
            end_cap_pts.push_back(ImVec2(end_pt.x + nx.x + ny.x, end_pt.y + nx.y + ny.y));
            end_cap_pts.push_back(ImVec2(end_pt.x - nx.x + ny.x, end_pt.y - nx.y + ny.y));
            end_cap_pts.push_back(ImVec2(end_pt.x - d.x, end_pt.y - d.y));
        } else if (cap == ImWidgetsCap_TriangleOut) {
            ImVec2 ny(-d.y, d.x); // tangent direction (length ~half_w)
            end_cap_pts.push_back(fwd[fwd.Size - 1]);
            end_cap_pts.push_back(ImVec2(end_pt.x + ny.x, end_pt.y + ny.y));
        } else if (cap == ImWidgetsCap_TriangleIn) {
            ImVec2 ny(-d.y, d.x);
            end_cap_pts.push_back(fwd[fwd.Size - 1]);
            end_cap_pts.push_back(ImVec2(end_pt.x - ny.x, end_pt.y - ny.y));
        }
        // Start cap
        if (cap == ImWidgetsCap_Round) {
            start_cap_pts.push_back(bwd[0]);
            DW_RoundCap(start_cap_pts, tol, start_pt, start_norm);
        } else if (cap == ImWidgetsCap_Square) {
            ImVec2 nx = start_norm, ny(-start_norm.y, start_norm.x);
            start_cap_pts.push_back(bwd[0]);
            start_cap_pts.push_back(ImVec2(start_pt.x + nx.x + ny.x, start_pt.y + nx.y + ny.y));
            start_cap_pts.push_back(ImVec2(start_pt.x - nx.x + ny.x, start_pt.y - nx.y + ny.y));
            start_cap_pts.push_back(ImVec2(start_pt.x - start_norm.x, start_pt.y - start_norm.y));
        } else if (cap == ImWidgetsCap_TriangleOut) {
            ImVec2 ny(-start_norm.y, start_norm.x); // tangent (backward, away from stroke)
            start_cap_pts.push_back(bwd[0]);
            start_cap_pts.push_back(ImVec2(start_pt.x + ny.x, start_pt.y + ny.y));
        } else if (cap == ImWidgetsCap_TriangleIn) {
            ImVec2 ny(-start_norm.y, start_norm.x);
            start_cap_pts.push_back(bwd[0]);
            start_cap_pts.push_back(ImVec2(start_pt.x - ny.x, start_pt.y - ny.y));
        }
    }

    // --- Build line soup (directed segments) ---
    ImVector<ImVec2> segs; // pairs: p0, p1, p0, p1, ...

    if (closed) {
        // Closed path: two independent closed loops (matching reference finish_closed).
        // Fwd loop (forward direction) — implicitly closed since closing join
        // brings fwd[last] back to fwd[0].
        for (int i = 0; i < fwd.Size - 1; ++i) {
            segs.push_back(fwd[i]);
            segs.push_back(fwd[i + 1]);
        }
        // Bwd loop (REVERSE direction) — implicitly closed.
        for (int i = bwd.Size - 1; i > 0; --i) {
            segs.push_back(bwd[i]);
            segs.push_back(bwd[i - 1]);
        }
        // No crossover segments — shader evaluates winding per-segment independently.
    } else {
        // Open path: single contour — fwd → end_cap → bwd(reversed) → start_cap → close
        for (int i = 0; i < fwd.Size - 1; ++i) {
            segs.push_back(fwd[i]);
            segs.push_back(fwd[i + 1]);
        }

        // End cap segments
        for (int i = 0; i < end_cap_pts.Size - 1; ++i) {
            segs.push_back(end_cap_pts[i]);
            segs.push_back(end_cap_pts[i + 1]);
        }
        if (end_cap_pts.Size > 0) {
            segs.push_back(end_cap_pts[end_cap_pts.Size - 1]);
            segs.push_back(bwd[bwd.Size - 1]);
        } else {
            // Butt/None: direct connection fwd.last → bwd.last
            segs.push_back(fwd[fwd.Size - 1]);
            segs.push_back(bwd[bwd.Size - 1]);
        }

        // bwd segments (REVERSE direction)
        for (int i = bwd.Size - 1; i > 0; --i) {
            segs.push_back(bwd[i]);
            segs.push_back(bwd[i - 1]);
        }

        // Start cap segments
        for (int i = 0; i < start_cap_pts.Size - 1; ++i) {
            segs.push_back(start_cap_pts[i]);
            segs.push_back(start_cap_pts[i + 1]);
        }
        if (start_cap_pts.Size > 0) {
            segs.push_back(start_cap_pts[start_cap_pts.Size - 1]);
            segs.push_back(fwd[0]);
        } else {
            // Butt/None: direct connection bwd[0] → fwd[0]
            segs.push_back(bwd[0]);
            segs.push_back(fwd[0]);
        }
    }

    int num_segs = segs.Size / 2;
    if (num_segs <= 0 || num_segs > IMGUI_STROKE_MAX_SEGMENTS) return;


    // --- Compute bounding box ---
    float aa = 1.0f; // AA fringe in pixels
    float bx0 = 1e9f, by0 = 1e9f, bx1 = -1e9f, by1 = -1e9f;
    for (int i = 0; i < segs.Size; ++i) {
        bx0 = ImMin(bx0, segs[i].x); by0 = ImMin(by0, segs[i].y);
        bx1 = ImMax(bx1, segs[i].x); by1 = ImMax(by1, segs[i].y);
    }
    bx0 -= aa; by0 -= aa; bx1 += aa; by1 += aa;

    // --- Fill constant buffer ---
    // Use pool to keep data alive until GPU reads it (double-buffered)
    ImU64 frame = ImGui::GetFrameCount();
    if (frame != s_strokeCBFrame) {
        s_strokeCBFrame = frame;
        // Free previous frame's pool
        for (int i = 0; i < s_strokeCBPool.Size; ++i) {
            if (s_strokeCBPool[i]->buf) IM_FREE(s_strokeCBPool[i]->buf);
            IM_FREE(s_strokeCBPool[i]);
        }
        s_strokeCBPool.resize(0);
    }

    int buf_size = (int)(12 * sizeof(float) + num_segs * 4 * sizeof(float)); // params+color+bounds + segments
    buf_size = (buf_size + 15) & ~15; // 16-byte align
    ImWidgetsStrokeBuffer* buf = (ImWidgetsStrokeBuffer*)IM_ALLOC(buf_size);
    memset(buf, 0, buf_size);

    buf->params[0] = (float)num_segs;
    buf->params[1] = half_w;
    buf->params[2] = aa;
    buf->params[3] = 0;

    // Color: convert ImU32 to float4
    buf->color[0] = ((col >> 0) & 0xFF) / 255.0f;
    buf->color[1] = ((col >> 8) & 0xFF) / 255.0f;
    buf->color[2] = ((col >> 16) & 0xFF) / 255.0f;
    buf->color[3] = ((col >> 24) & 0xFF) / 255.0f;

    buf->bounds[0] = bx0; buf->bounds[1] = by0;
    buf->bounds[2] = bx1; buf->bounds[3] = by1;

    for (int i = 0; i < num_segs; ++i) {
        buf->segments[i * 4 + 0] = segs[i * 2].x;
        buf->segments[i * 4 + 1] = segs[i * 2].y;
        buf->segments[i * 4 + 2] = segs[i * 2 + 1].x;
        buf->segments[i * 4 + 3] = segs[i * 2 + 1].y;
    }

    DW_StrokeCBData* cb = (DW_StrokeCBData*)IM_ALLOC(sizeof(DW_StrokeCBData));
    cb->program = program;
    cb->buf = buf;
    cb->buf_size = buf_size;
    s_strokeCBPool.push_back(cb);

    // --- Render bounding quad via custom shader ---
    dl->AddCallback(DW_PrepareStrokeUniforms, cb);
    ImPlatform_BeginCustomShader(dl, program);
    dl->AddImageQuad(
        gs_pContext->whiteImg,
        ImVec2(bx0, by0), ImVec2(bx1, by0), ImVec2(bx1, by1), ImVec2(bx0, by1),
        ImVec2(0, 0), ImVec2(1, 0), ImVec2(1, 1), ImVec2(0, 1),
        IM_COL32(255, 255, 255, 255));
    ImPlatform_EndCustomShader(dl);

    // --- Debug wireframe overlay ---
    if (s_strokeDebugWireframe) {
        ImU32 cL = IM_COL32(255, 100, 100, 200), cR = IM_COL32(100, 100, 255, 200);
        for (int i = 0; i < fwd.Size - 1; ++i) dl->AddLine(fwd[i], fwd[i + 1], cL, 1);
        for (int i = 0; i < bwd.Size - 1; ++i) dl->AddLine(bwd[i], bwd[i + 1], cR, 1);
    }
}


// ============================================================
// Dash splitting — preserves cubic representation for smooth strokes.
// Arc-length is measured via Euler sub-segment chord lengths, then
// dash boundaries are mapped back to cubic t-parameters for De Casteljau split.
// ============================================================

// Build cumulative arc-length table for one cubic via Euler sub-segments.
// Returns pairs of (t_in_cubic, cumulative_arc_length).
struct DW_ArcLenEntry { float t; float len; };
static float DW_BuildArcLenTable(ImVec2 c0, ImVec2 c1, ImVec2 c2, ImVec2 c3, float tol,
    ImVector<DW_ArcLenEntry>& table)
{
    // Walk CubicToEuler subdivision to get (t, arc_len) pairs
    ImVec2 lp=c0, lq(c1.x-c0.x,c1.y-c0.y);
    if(lq.x*lq.x+lq.y*lq.y<1e-12f){ImVec2 tmp; DW_CubicEvalAndDeriv(c0,c1,c2,c3,1e-6f,tmp,lq);}
    float lt=0, cum_len=0; ImU64 t0_u=0; float dt=1;
    DW_ArcLenEntry e0={0,0}; table.push_back(e0);
    for(int iter=0;iter<10000;++iter){
        float t0f=(float)t0_u*dt; if(t0f>=1)break;
        float t1=t0f+dt; if(t1>1)t1=1;
        ImVec2 p1,q1; DW_CubicEvalAndDeriv(c0,c1,c2,c3,t1,p1,q1);
        if(q1.x*q1.x+q1.y*q1.y<1e-12f){
            ImVec2 pb,qb; DW_CubicEvalAndDeriv(c0,c1,c2,c3,t1-1e-6f,pb,qb);
            q1=qb; if(t1<1){p1=pb;t1-=1e-6f;}
        }
        DW_CubicParams cp=DW_CubicParams::FromPointsDerivs(lp,p1,lq,q1,t1-lt);
        if(cp.err*cp.chord_len<=tol || dt<1e-6f){
            cum_len += cp.chord_len;
            DW_ArcLenEntry e={t1, cum_len}; table.push_back(e);
            lp=p1; lq=q1; lt=t1;
            t0_u+=1;
            if(t0_u>0){unsigned sh=0;ImU64 tmp=t0_u;while((tmp&1)==0){sh++;tmp>>=1;}t0_u>>=sh;dt*=(float)(1ULL<<sh);}
        } else { t0_u*=2; dt*=0.5f; }
    }
    return cum_len;
}

// Find cubic t-parameter for a given arc length using the table.
static float DW_ArcLenToT(const ImVector<DW_ArcLenEntry>& table, float target_len)
{
    if (table.Size < 2) return 0;
    if (target_len <= 0) return 0;
    if (target_len >= table[table.Size-1].len) return 1;
    // Binary search
    int lo = 0, hi = table.Size - 1;
    while (lo < hi - 1) {
        int mid = (lo + hi) / 2;
        if (table[mid].len < target_len) lo = mid; else hi = mid;
    }
    float frac = (target_len - table[lo].len) / (table[hi].len - table[lo].len + 1e-9f);
    return table[lo].t + frac * (table[hi].t - table[lo].t);
}

// Dash-split a cubic Bezier path, preserving cubic representation.
// out_dashes: each entry is 3*N+1 control points for N cubics.
static void DW_DashSplitCubicPath(
    const ImVec2* cubics, int n_cubics, float tol, bool closed,
    const float* dash_array, int dash_count, float dash_offset,
    ImVector<ImVector<ImVec2>>& out_dashes)
{
    if (dash_count <= 0 || n_cubics <= 0) return;
    float pattern_len = 0;
    for (int i = 0; i < dash_count; ++i) pattern_len += dash_array[i];
    if (pattern_len <= 0) return;

    // Build extended cubic list (add closing line as degenerate cubic if closed)
    ImVector<ImVec2> all_pts;
    for (int i = 0; i <= n_cubics * 3; ++i) all_pts.push_back(cubics[i]);
    int total_cubics = n_cubics;
    if (closed) {
        ImVec2 last = cubics[n_cubics * 3], first = cubics[0];
        float dx = first.x-last.x, dy = first.y-last.y;
        if (dx*dx+dy*dy > 1e-6f) {
            // Add closing segment as degenerate cubic (straight line)
            ImVec2 m1(last.x+dx*0.333f, last.y+dy*0.333f);
            ImVec2 m2(last.x+dx*0.667f, last.y+dy*0.667f);
            all_pts.push_back(m1); all_pts.push_back(m2); all_pts.push_back(first);
            total_cubics++;
        }
    }

    // Normalize dash offset
    float off = fmodf(dash_offset, pattern_len);
    if (off < 0) off += pattern_len;
    int dash_idx = 0;
    float remain = dash_array[0];
    while (off > 0) {
        if (off < remain) { remain -= off; break; }
        off -= remain;
        dash_idx = (dash_idx + 1) % dash_count;
        remain = dash_array[dash_idx];
    }
    bool in_dash = (dash_idx % 2) == 0;

    ImVector<ImVec2> cur_dash; // current dash sub-path (3*N+1 control points)

    for (int ci = 0; ci < total_cubics; ++ci) {
        ImVec2 c0=all_pts[ci*3], c1=all_pts[ci*3+1], c2=all_pts[ci*3+2], c3=all_pts[ci*3+3];

        // Build arc-length table for this cubic
        ImVector<DW_ArcLenEntry> arc_table;
        float cubic_len = DW_BuildArcLenTable(c0, c1, c2, c3, tol, arc_table);
        if (cubic_len < 1e-6f) continue;

        // Current remaining cubic (gets subdivided as we split)
        ImVec2 rem[4] = {c0, c1, c2, c3};
        float rem_arc_start = 0; // arc length consumed so far within this cubic

        while (rem_arc_start < cubic_len - 1e-6f) {
            if (remain <= 0) {
                dash_idx = (dash_idx + 1) % dash_count;
                remain = dash_array[dash_idx];
                in_dash = (dash_idx % 2) == 0;
            }

            float rem_len = cubic_len - rem_arc_start;
            if (remain >= rem_len - 1e-4f) {
                // Entire remaining cubic fits
                if (in_dash) {
                    if (cur_dash.Size == 0) cur_dash.push_back(rem[0]);
                    cur_dash.push_back(rem[1]); cur_dash.push_back(rem[2]); cur_dash.push_back(rem[3]);
                } else if (cur_dash.Size >= 4) {
                    out_dashes.push_back(ImVector<ImVec2>());
                    out_dashes[out_dashes.Size-1].swap(cur_dash);
                    cur_dash.resize(0);
                }
                remain -= rem_len;
                rem_arc_start = cubic_len;
            } else {
                // Split within this cubic
                float target_arc = rem_arc_start + remain;
                float t_global = DW_ArcLenToT(arc_table, target_arc);
                // Convert global t to local t within remaining sub-cubic
                float t0_global = DW_ArcLenToT(arc_table, rem_arc_start);
                float t_local = (t0_global < 1.f - 1e-6f) ?
                    (t_global - t0_global) / (1.f - t0_global) : 0.5f;
                t_local = ImClamp(t_local, 0.001f, 0.999f);

                ImVec2 left[4], right[4];
                DW_CubicSubdivide(rem[0], rem[1], rem[2], rem[3], t_local, left, right);

                if (in_dash) {
                    if (cur_dash.Size == 0) cur_dash.push_back(left[0]);
                    cur_dash.push_back(left[1]); cur_dash.push_back(left[2]); cur_dash.push_back(left[3]);
                    out_dashes.push_back(ImVector<ImVec2>());
                    out_dashes[out_dashes.Size-1].swap(cur_dash);
                    cur_dash.resize(0);
                }

                rem_arc_start += remain;
                remain = 0;
                rem[0]=right[0]; rem[1]=right[1]; rem[2]=right[2]; rem[3]=right[3];
            }
        }
    }

    if (in_dash && cur_dash.Size >= 4) {
        out_dashes.push_back(ImVector<ImVec2>());
        out_dashes[out_dashes.Size-1].swap(cur_dash);
    }
}

// Split a polyline at dash/gap boundaries.
static void DW_DashSplitPolyline(
    const ImVec2* pts, int n_pts, bool closed,
    const float* dash_array, int dash_count, float dash_offset,
    ImVector<ImVector<ImVec2>>& out_dashes)
{
    if (dash_count <= 0 || n_pts < 2) return;
    float pattern_len = 0;
    for (int i = 0; i < dash_count; ++i) pattern_len += dash_array[i];
    if (pattern_len <= 0) return;

    // Build extended point list for closed paths
    ImVector<ImVec2> poly;
    for (int i = 0; i < n_pts; ++i) poly.push_back(pts[i]);
    if (closed) {
        ImVec2 first=poly[0], last=poly[poly.Size-1];
        float dx=first.x-last.x, dy=first.y-last.y;
        if (dx*dx+dy*dy > 1e-6f) poly.push_back(first);
    }

    float off = fmodf(dash_offset, pattern_len);
    if (off < 0) off += pattern_len;
    int dash_idx = 0;
    float remain = dash_array[0];
    while (off > 0) {
        if (off < remain) { remain -= off; break; }
        off -= remain;
        dash_idx = (dash_idx + 1) % dash_count;
        remain = dash_array[dash_idx];
    }
    bool in_dash = (dash_idx % 2) == 0;

    ImVector<ImVec2> cur_dash;
    if (in_dash) cur_dash.push_back(poly[0]);

    for (int i = 0; i < poly.Size - 1; ++i) {
        ImVec2 a = poly[i], b = poly[i + 1];
        float seg_dx = b.x-a.x, seg_dy = b.y-a.y;
        float seg_len = sqrtf(seg_dx*seg_dx + seg_dy*seg_dy);
        if (seg_len < 1e-6f) continue;

        float consumed = 0;
        while (consumed < seg_len - 1e-6f) {
            if (remain <= 0) {
                dash_idx = (dash_idx + 1) % dash_count;
                remain = dash_array[dash_idx];
                in_dash = (dash_idx % 2) == 0;
                if (in_dash) {
                    float t = consumed / seg_len;
                    cur_dash.push_back(ImVec2(a.x+seg_dx*t, a.y+seg_dy*t));
                }
            }
            float left = seg_len - consumed;
            if (remain >= left - 1e-4f) {
                if (in_dash) cur_dash.push_back(b);
                remain -= left;
                consumed = seg_len;
            } else {
                consumed += remain;
                float t = consumed / seg_len;
                ImVec2 split(a.x+seg_dx*t, a.y+seg_dy*t);
                if (in_dash) {
                    cur_dash.push_back(split);
                    if (cur_dash.Size >= 2) {
                        out_dashes.push_back(ImVector<ImVec2>());
                        out_dashes[out_dashes.Size-1].swap(cur_dash);
                    }
                    cur_dash.resize(0);
                }
                remain = 0;
            }
        }
    }
    if (in_dash && cur_dash.Size >= 2) {
        out_dashes.push_back(ImVector<ImVec2>());
        out_dashes[out_dashes.Size-1].swap(cur_dash);
    }
}

// ============================================================
// Public API
// ============================================================

// Helper: compute start_norm from first tangent
static ImVec2 DW_ComputeNorm(ImVec2 tan, float half_w) {
    float tl = sqrtf(tan.x*tan.x + tan.y*tan.y);
    if (tl < 1e-6f) return ImVec2(0, half_w);
    return ImVec2(-tan.y/tl*half_w, tan.x/tl*half_w);
}

void DrawStrokedCubicBezier(ImDrawList* drawlist,
    ImVec2 p0, ImVec2 p1, ImVec2 p2, ImVec2 p3,
    ImU32 col, float thickness, ImWidgetsCap cap, float tolerance,
    ImWidgetsPrimitive /*primitive*/, ImWidgetsCorrectness correctness)
{
    if (!drawlist||thickness<=0||(col&IM_COL32_A_MASK)==0) return;
    float hw = thickness * 0.5f;
    bool strong = (correctness == ImWidgetsCorrectness_Strong);
    ImVec2 pts[4] = {p0, p1, p2, p3};
    ImVector<ImVec2> fwd, bwd;
    DW_StrokeBuildOutlineCubics(pts, 1, hw, tolerance, cap, ImWidgetsJoin_Round, 4.f, false, strong, fwd, bwd);
    ImVec2 ts(p1.x-p0.x, p1.y-p0.y);
    ImVec2 start_norm = DW_ComputeNorm(ts, hw);
    DW_StrokeRenderOutline(drawlist, fwd, bwd, col, hw, cap, false, tolerance, p0, p3, start_norm);
}

void DrawStrokedBezierPath(ImDrawList* drawlist,
    const ImVec2* points, int points_count, ImU32 col, float thickness,
    ImWidgetsCap cap, ImWidgetsJoin join, float miter_limit, float tolerance, bool closed,
    ImWidgetsPrimitive /*primitive*/, ImWidgetsCorrectness correctness)
{
    if (!drawlist||!points||thickness<=0||(col&IM_COL32_A_MASK)==0) return;
    int nc = (points_count-1)/3; if (nc <= 0) return;
    float hw = thickness * 0.5f;
    bool strong = (correctness == ImWidgetsCorrectness_Strong);
    ImVector<ImVec2> fwd, bwd;
    DW_StrokeBuildOutlineCubics(points, nc, hw, tolerance, cap, join, miter_limit, closed, strong, fwd, bwd);
    ImVec2 ts(points[1].x-points[0].x, points[1].y-points[0].y);
    ImVec2 start_norm = DW_ComputeNorm(ts, hw);
    int lb = (nc-1)*3;
    DW_StrokeRenderOutline(drawlist, fwd, bwd, col, hw, cap, closed, tolerance, points[0], points[lb+3], start_norm);
}

void DrawStrokedPolyline(ImDrawList* drawlist,
    const ImVec2* points, int points_count, ImU32 col, float thickness,
    ImWidgetsCap cap, ImWidgetsJoin join, float miter_limit, bool closed)
{
    if (!drawlist||!points||points_count<2||thickness<=0||(col&IM_COL32_A_MASK)==0) return;
    float hw = thickness * 0.5f;
    float tol = 0.25f;
    ImVector<ImVec2> fwd, bwd;
    DW_StrokeBuildOutlinePolyline(points, points_count, hw, tol, cap, join, miter_limit, closed, fwd, bwd);
    ImVec2 ts(points[1].x-points[0].x, points[1].y-points[0].y);
    ImVec2 start_norm = DW_ComputeNorm(ts, hw);
    ImVec2 end_pt = closed ? points[0] : points[points_count-1];
    DW_StrokeRenderOutline(drawlist, fwd, bwd, col, hw, cap, closed, tol, points[0], end_pt, start_norm);
}

void DrawStrokedDashedBezierPath(ImDrawList* drawlist,
    const ImVec2* points, int points_count, ImU32 col, float thickness,
    const float* dash_array, int dash_count, float dash_offset,
    ImWidgetsCap cap, ImWidgetsJoin join, float miter_limit, float tolerance,
    bool closed,
    ImWidgetsPrimitive primitive, ImWidgetsCorrectness correctness)
{
    if (!drawlist||!points||thickness<=0||(col&IM_COL32_A_MASK)==0) return;
    int nc = (points_count-1)/3; if (nc <= 0) return;
    if (!dash_array || dash_count <= 0) {
        DrawStrokedBezierPath(drawlist, points, points_count, col, thickness,
            cap, join, miter_limit, tolerance, closed, primitive, correctness);
        return;
    }

    // Split cubic path at dash boundaries, preserving cubic representation
    ImVector<ImVector<ImVec2>> dashes;
    DW_DashSplitCubicPath(points, nc, tolerance, closed, dash_array, dash_count, dash_offset, dashes);

    // Stroke each dash as a cubic path (smooth Euler spiral offset)
    for (int i = 0; i < dashes.Size; ++i) {
        ImVector<ImVec2>& dp = dashes[i];
        if (dp.Size < 4) continue;
        DrawStrokedBezierPath(drawlist, dp.Data, dp.Size, col, thickness,
            cap, join, miter_limit, tolerance, false, primitive, correctness);
    }
}

void DrawStrokedDashedPolyline(ImDrawList* drawlist,
    const ImVec2* points, int points_count, ImU32 col, float thickness,
    const float* dash_array, int dash_count, float dash_offset,
    ImWidgetsCap cap, ImWidgetsJoin join, float miter_limit, bool closed)
{
    if (!drawlist||!points||points_count<2||thickness<=0||(col&IM_COL32_A_MASK)==0) return;
    if (!dash_array || dash_count <= 0) {
        DrawStrokedPolyline(drawlist, points, points_count, col, thickness,
            cap, join, miter_limit, closed);
        return;
    }

    ImVector<ImVector<ImVec2>> dashes;
    DW_DashSplitPolyline(points, points_count, closed, dash_array, dash_count, dash_offset, dashes);

    for (int i = 0; i < dashes.Size; ++i) {
        ImVector<ImVec2>& dp = dashes[i];
        if (dp.Size < 2) continue;
        DrawStrokedPolyline(drawlist, dp.Data, dp.Size, col, thickness,
            cap, join, miter_limit, false);
    }
}

} // namespace ImWidgets

#endif // _DEAR_WIDGETS_STROKE_INCLUDED
