#include <imgui.h>

//#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>

#include <algorithm>
#include <string>

#include <cmath>

//////////////////////////////////////////////////////////////////////////
// Style TODO:
//	* Expose Style
//	* Line Thickness for Slider2D
//	* Line Color for Slider2D
//
// Known issue:
//	* Slider2DInt must take into account the half pixel
//	* Move 2D must store state "IsDrag"
//
// Optim TODO:
//	* ChromaticPlot draw only the internal MultiColorQuad where its needed "inside" or at least leave the option for style "transparency etc"
//	* ChromaticPlot: Bake some as much as possible values: provide different version: ChromaticPlotDynamic {From enum and compute info at each frame}, ChromaticPlotFromData {From Baked data}
//
// Write use case for:
//	* HueToHue:
//		- Color Remap
//	* LumToSat:
//		- Color Remap
//	* ColorRing:
//		- HDR Color Management {Shadow, MidTone, Highlight}
//	* Grid2D_AoS_Float:
//		- Color Remap:
//////////////////////////////////////////////////////////////////////////

#if !__cpp_if_constexpr
#define constexpr
#endif

#if 0
#define nullptr NULL
#endif

namespace ImWidgets{
	typedef ImU32( *ImColor1DCallback )( float x, void* );
	typedef ImU32( *ImColor2DCallback )( float x, float y, void* );

	enum StyleColor
	{
		StyleColor_Value,

		StyleColor_Count
	};

	enum StyleVar
	{
		StyleVar_Value,

		StyleVar_Count
	};

	struct Style
	{
		float	Value;
		ImVec4  Colors[ StyleColor_Count ];

		Style()
		{
			Value = 1.0f;
			Colors[ StyleColor_Value ] = ImVec4( 1.0f, 0.0f, 0.0f, 1.0f );
		}

		void PushColor( StyleColor colorIndex, const ImVec4& color )
		{
			ColorModifier modifier;
			modifier.Index = colorIndex;
			modifier.Value = Colors[ colorIndex ];
			m_ColorStack.push_back( modifier );
			Colors[ colorIndex ] = color;
		}
		void PopColor( int count = 1 )
		{
			while ( count > 0 )
			{
				auto& modifier = m_ColorStack.back();
				Colors[ modifier.Index ] = modifier.Value;
				m_ColorStack.pop_back();
				--count;
			}
		}

		void PushVar( StyleVar varIndex, float value )
		{
			auto* var = GetVarFloatAddr( varIndex );
			IM_ASSERT( var != nullptr );
			VarModifier modifier;
			modifier.Index = varIndex;
			modifier.Value = ImVec4( *var, 0, 0, 0 );
			*var = value;
			m_VarStack.push_back( modifier );
		}
		void PushVar( StyleVar varIndex, const ImVec2& value )
		{
			auto* var = GetVarVec2Addr( varIndex );
			IM_ASSERT( var != nullptr );
			VarModifier modifier;
			modifier.Index = varIndex;
			modifier.Value = ImVec4( var->x, var->y, 0, 0 );
			*var = value;
			m_VarStack.push_back( modifier );
		}
		void PushVar( StyleVar varIndex, const ImVec4& value )
		{
			auto* var = GetVarVec4Addr( varIndex );
			IM_ASSERT( var != nullptr );
			VarModifier modifier;
			modifier.Index = varIndex;
			modifier.Value = *var;
			*var = value;
			m_VarStack.push_back( modifier );
		}
		void PopVar( int count = 1 )
		{
			while ( count > 0 )
			{
				auto& modifier = m_VarStack.back();
				if ( auto floatValue = GetVarFloatAddr( modifier.Index ) )
					*floatValue = modifier.Value.x;
				else if ( auto vec2Value = GetVarVec2Addr( modifier.Index ) )
					*vec2Value = ImVec2( modifier.Value.x, modifier.Value.y );
				else if ( auto vec4Value = GetVarVec4Addr( modifier.Index ) )
					*vec4Value = modifier.Value;
				m_VarStack.pop_back();
				--count;
			}
		}

		const char* GetColorName( StyleColor colorIndex ) const
		{
			switch ( colorIndex )
			{
			case StyleColor_Value: return "Value";
			case StyleColor_Count: break;
			}

			IM_ASSERT( 0 );
			return "Unknown";
		}

	private:
		struct ColorModifier
		{
			StyleColor  Index;
			ImVec4      Value;
		};

		struct VarModifier
		{
			StyleVar Index;
			ImVec4   Value;
		};

		float* GetVarFloatAddr( StyleVar idx )
		{
			switch ( idx )
			{
			case StyleVar_Value:	return &Value;
			default:				return nullptr;
			}
		}
		ImVec2* GetVarVec2Addr( StyleVar idx )
		{
			return NULL;
		}
		ImVec4* GetVarVec4Addr( StyleVar idx )
		{
			return NULL;
		}

		ImVector<ColorModifier>	m_ColorStack;
		ImVector<VarModifier>	m_VarStack	;
	};

	Style& GetStyle();

	inline
	const char* GetStyleColorName( StyleColor colorIndex )
	{
		GetStyle().GetColorName( colorIndex );
	}
	inline
	void PushStyleColor( StyleColor colorIndex, const ImVec4& color )
	{
		GetStyle().PushColor( colorIndex, color );
	}
	inline
	void PopStyleColor( int count = 1 )
	{
		GetStyle().PopColor( count );
	}
	inline
	void PushStyleVar( StyleVar varIndex, float value )
	{
		GetStyle().PushVar( varIndex, value );
	}
	inline
	void PushStyleVar( StyleVar varIndex, const ImVec2& value )
	{
		GetStyle().PushVar( varIndex, value );
	}
	inline
	void PushStyleVar( StyleVar varIndex, const ImVec4& value )
	{
		GetStyle().PushVar( varIndex, value );
	}
	inline
	void PopStyleVar( int count = 1 )
	{
		GetStyle().PopVar( count );
	}

#define ImWidgets_Kibi (1024ull)
#define ImWidgets_Mibi (ImWidgets_Kibi*1024ull)
#define ImWidgets_Gibi (ImWidgets_Mibi*1024ull)
#define ImWidgets_Tebi (ImWidgets_Gibi*1024ull)
#define ImWidgets_Pebi (ImWidgets_Tebi*1024ull)

	typedef int ImWidgetsLengthUnit;
	typedef int ImWidgetsChromaticPlot;
	typedef int ImWidgetsObserver;
	typedef int ImWidgetsIlluminance;
	typedef int ImWidgetsColorSpace;
	typedef int ImWidgetsPointer;

	enum ImWidgetsLengthUnit_
	{
		ImWidgetsLengthUnit_Metric = 0,
		ImWidgetsLengthUnit_Imperial,
		ImWidgetsLengthUnit_COUNT
	};

	enum ImWidgetsObserver_
	{
		// Standard
		ImWidgetsObserverChromaticPlot_1931_2deg = 0,
		ImWidgetsObserverChromaticPlot_1964_10deg,
		ImWidgetsObserverChromaticPlot_COUNT
	};

	enum ImWidgetsIlluminance_
	{
		// White Points
		ImWidgetsWhitePointChromaticPlot_A = 0,
		ImWidgetsWhitePointChromaticPlot_B,
		ImWidgetsWhitePointChromaticPlot_C,
		ImWidgetsWhitePointChromaticPlot_D50,
		ImWidgetsWhitePointChromaticPlot_D55,
		ImWidgetsWhitePointChromaticPlot_D65,
		ImWidgetsWhitePointChromaticPlot_D75,
		ImWidgetsWhitePointChromaticPlot_D93,
		ImWidgetsWhitePointChromaticPlot_E,
		ImWidgetsWhitePointChromaticPlot_F1,
		ImWidgetsWhitePointChromaticPlot_F2,
		ImWidgetsWhitePointChromaticPlot_F3,
		ImWidgetsWhitePointChromaticPlot_F4,
		ImWidgetsWhitePointChromaticPlot_F5,
		ImWidgetsWhitePointChromaticPlot_F6,
		ImWidgetsWhitePointChromaticPlot_F7,
		ImWidgetsWhitePointChromaticPlot_F8,
		ImWidgetsWhitePointChromaticPlot_F9,
		ImWidgetsWhitePointChromaticPlot_F10,
		ImWidgetsWhitePointChromaticPlot_F11,
		ImWidgetsWhitePointChromaticPlot_F12,
		ImWidgetsWhitePointChromaticPlot_COUNT
	};

	enum ImWidgetsColorSpace_
	{
		// Color Spaces
		ImWidgetsColorSpace_AdobeRGB = 0,	// D65
		ImWidgetsColorSpace_AppleRGB,		// D65
		ImWidgetsColorSpace_Best,			// D50
		ImWidgetsColorSpace_Beta,			// D50
		ImWidgetsColorSpace_Bruce,			// D65
		ImWidgetsColorSpace_CIERGB,			// E
		ImWidgetsColorSpace_ColorMatch,		// D50
		ImWidgetsColorSpace_Don_RGB_4,		// D50
		ImWidgetsColorSpace_ECI,			// D50
		ImWidgetsColorSpace_Ekta_Space_PS5,	// D50
		ImWidgetsColorSpace_NTSC,			// C
		ImWidgetsColorSpace_PAL_SECAM,		// D65
		ImWidgetsColorSpace_ProPhoto,		// D50
		ImWidgetsColorSpace_SMPTE_C,		// D65
		ImWidgetsColorSpace_sRGB,			// D65
		ImWidgetsColorSpace_WideGamutRGB,	// D50
		ImWidgetsColorSpace_Rec2020,		// D65
		ImWidgetsColorSpace_COUNT
	};

	enum ImWidgetsChromaticPlot_
	{
		// Style
		ImWidgetsChromaticPlot_ShowWavelength,
		ImWidgetsChromaticPlot_ShowGrid,
		ImWidgetsChromaticPlot_ShowPrimaries,
		ImWidgetsChromaticPlot_ShowWhitePoint,

		ImWidgetsChromaticPlot_COUNT
	};

	//////////////////////////////////////////////////////////////////////////
	// Helpers
	//////////////////////////////////////////////////////////////////////////
	inline
	float ImFract(float x)
	{
		float iPart;
		return ImFmod(x, iPart);
	}

	inline
	float ImSmoothStep(float edge0, float edge1, float x)
	{
		// Scale, bias and saturate x to 0..1 range
		x = ImClamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
		// Evaluate polynomial
		return x * x * (3.0f - 2.0f * x);
	}

	inline
	float ImDot( ImVec4 const& a, ImVec4 const& b )
	{
		return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
	}
	inline
	float	ImNormalize01(float const x, float const _min, float const _max)
	{
		return ( x - _min ) / ( _max - _min );
	}
	inline
	float	ImScaleFromNormalized(float const x, float const newMin, float const newMax)
	{
		return x * ( newMax - newMin ) + newMin;
	}
	inline
	float	ImRescale(float const x, float const _min, float const _max, float const newMin, float const newMax)
	{
		return ImScaleFromNormalized( ImNormalize01( x, _min, _max ), newMin, newMax );
	}
	template < typename Type >
	inline
	Type	Normalize01(Type const x, Type const _min, Type const _max)
	{
		return ( x - _min ) / ( _max - _min );
	}
	template < typename Type >
	inline
	Type	ScaleFromNormalized(Type const x, Type const newMin, Type const newMax)
	{
		return x * ( newMax - newMin ) + newMin;
	}
	template < typename Type >
	inline
	Type	Rescale(Type const x, Type const _min, Type const _max, Type const newMin, Type const newMax)
	{
		return ScaleFromNormalized( Normalize01( x, _min, _max ), newMin, newMax );
	}

	inline
	float ImLength(ImVec2 v)
	{
		return ImSqrt( ImLengthSqr( v ) );
	}
	inline
	float ImLength(ImVec4 v)
	{
		return ImSqrt( ImLengthSqr( v ) );
	}
	float	ImLinearSample( float t, float* buffer, int count );
	inline
	float	FunctionFromData( float const x, float const minX, float const maxX, float* data, int const samples_count )
	{
		float const t = ImSaturate( ImNormalize01( x, minX, maxX ) );

		return ImLinearSample( t, data, samples_count );
	}

	//////////////////////////////////////////////////////////////////////////
	// Color Functions
	//////////////////////////////////////////////////////////////////////////
	IMGUI_API ImU32	ImColorFrom_xyz( float x, float y, float z, float* xyzToRGB, float gamma );

	//////////////////////////////////////////////////////////////////////////
	// Scalar Helpers
	//////////////////////////////////////////////////////////////////////////
	void	ScaleData( ImGuiDataType data_type, void* p_data, double value );
	void	ScaleData( ImGuiDataType data_type, void* p_data, ImU64 value );
	bool	IsNegativeScalar( ImGuiDataType data_type, ImU64* src );
	bool	IsPositiveScalar( ImGuiDataType data_type, ImU64* src );
	void	EqualScalar( ImGuiDataType data_type, ImU64* p_target, ImU64* p_source );
	float	ScalarToFloat( ImGuiDataType data_type, ImU64* p_source );
	ImU64	FloatToScalar( ImGuiDataType data_type, float f_value );
	ImU64	AddScalar( ImGuiDataType data_type, void* p_a, void* p_b );
	ImU64	SubScalar( ImGuiDataType data_type, void* p_a, void* p_b );
	ImU64	MulScalar( ImGuiDataType data_type, void* p_a, void* p_b );
	ImU64	DivScalar( ImGuiDataType data_type, void* p_a, void* p_b );
	ImU64	ClampScalar( ImGuiDataType data_type, void* p_value, void* p_min, void* p_max );
	ImU64	Normalize01( ImGuiDataType data_type, void* p_value, void const* p_min, void const* p_max );
	void	MemoryString( std::string& sResult, ImU64 const uMemoryByte );
	void	MemoryString( std::string& sResult, ImU64 const uMemoryByte );
	void	MemoryString( std::string& sResult, ImU64 const uMemoryByte );

	//////////////////////////////////////////////////////////////////////////
	// DrawList
	//////////////////////////////////////////////////////////////////////////
	IMGUI_API void DrawTrianglePointer( ImDrawList* pDrawList, ImVec2 targetPoint, float angle, float size, float thickness, ImU32 col );
	IMGUI_API void DrawTrianglePointerFilled( ImDrawList* pDrawList, ImVec2 targetPoint, float angle, float size, ImU32 col );

	IMGUI_API void DrawProceduralColor1DNearest( ImDrawList* pDrawList, ImColor1DCallback func, void* pUserData, float minX, float maxX, ImVec2 position, ImVec2 size, int resolutionX );
	IMGUI_API void DrawProceduralColor1DBilinear( ImDrawList* pDrawList, ImColor1DCallback func, void* pUserData, float minX, float maxX, ImVec2 position, ImVec2 size, int resolutionX );

	IMGUI_API void DrawProceduralColor2DNearest( ImDrawList* pDrawList, ImColor2DCallback func, void* pUserData, float minX, float maxX, float minY, float maxY, ImVec2 position, ImVec2 size, int resolutionX, int resolutionY );
	IMGUI_API void DrawProceduralColor2DBilinear( ImDrawList* pDrawList, ImColor2DCallback func, void* pUserData, float minX, float maxX, float minY, float maxY, ImVec2 position, ImVec2 size, int resolutionX, int resolutionY );

	IMGUI_API void DrawHueBand( ImDrawList* pDrawList, ImVec2 const vpos, ImVec2 const size, int division, float alpha, float gamma, float offset );
	IMGUI_API void DrawHueBand( ImDrawList* pDrawList, ImVec2 const vpos, ImVec2 const size, int division, float colorStartRGB[ 3 ], float alpha, float gamma );
	IMGUI_API void DrawLumianceBand( ImDrawList* pDrawList, ImVec2 const vpos, ImVec2 const size, int division, ImVec4 const& color, float gamma );
	IMGUI_API void DrawSaturationBand( ImDrawList* pDrawList, ImVec2 const vpos, ImVec2 const size, int division, ImVec4 const& color, float gamma );

	IMGUI_API void DrawColorRing( ImDrawList* pDrawList, ImVec2 const curPos, ImVec2 const size, float thickness_, ImColor1DCallback func, void* pUserData, int division, float colorOffset, bool bIsBilinear );

	// poly: Counterclockwise: Positive shape & Clockwise for hole, don't forget to close your shape
	IMGUI_API void DrawShapeWithHole( ImDrawList* draw, ImVec2* poly, int points_count, ImColor color, int gap = 1, int strokeWidth = 1 );

	IMGUI_API
	void	DrawchromaticityPlotGeneric( ImDrawList* pDrawList,
										 ImVec2 const curPos,
										 float width, float height,
										 ImVec2 primR, ImVec2 primG, ImVec2 primB,
										 ImVec2 whitePoint,
										 float* xyzToRGB,
										 int const chromeLineSamplesCount,
										 float* observerX, float* observerY, float* observerZ,
										 int const observerSampleCount,
										 float const observerWavelengthMin, float const observerWavelengthMax,
										 float* standardCIE,
										 int const standardCIESampleCount,
										 float const standardCIEWavelengthMin, float const standardCIEWavelengthMax,
										 float gamma,
										 int resX, int resY,
										 ImU32 maskColor,
										 float wavelengthMin = 400.0f, float wavelengthMax = 700.0f,
										 float minX = 0.0f, float maxX = 0.8f,
										 float minY = 0.0f, float maxY = 0.9f );
	IMGUI_API void DrawchromaticityPlot( ImDrawList* draw,
										 ImWidgetsIlluminance illuminance,
										 ImWidgetsObserver observer,
										 ImWidgetsColorSpace colorSpace,
										 int chromeLineSamplesCount,
										 ImVec2 const vpos, ImVec2 const size,
										 int resolutionX, int resolutionY,
										 ImU32 maskColor,
										 float wavelengthMin = 400.0f, float wavelengthMax = 700.0f,
										 float minX = 0.0f, float maxX = 0.8f,
										 float minY = 0.0f, float maxY = 0.9f );

	//////////////////////////////////////////////////////////////////////////
	// Widgets
	//////////////////////////////////////////////////////////////////////////
}
