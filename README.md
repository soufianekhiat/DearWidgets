# DearWidgets
Collection of draw (from ImDrawList) and widgets I use.
Version 0.0.0.0.1

PR & Discussion are open.

## Features
### Draw
* Triangle Pointer {Right, Up, Left, Down}

Used internally for LineSlider

![](https://media.githubusercontent.com/media/soufianekhiat/DearWidgetsImages/main/Images/dearwidgetsdemo_mRxPnn8bNH.png)

* Hue Band
* Luminance Band
* Saturation Band

Used Internally to implement HueSelector.

![](https://media.githubusercontent.com/media/soufianekhiat/DearWidgetsImages/main/Images/dearwidgetsdemo_mw6vQsfBi7.png)
![](https://media.githubusercontent.com/media/soufianekhiat/DearWidgetsImages/main/Images/dearwidgetsdemo_4ufS2JkG81.png)

* Color Ring

TODO: Ring HueSelector
TODO: Add support for 2D (angle, radius) lambda

![](https://media.githubusercontent.com/media/soufianekhiat/DearWidgetsImages/main/Images/GQLfC3C7Jk.gif)

* Chromatic Plot{Bilinear, Nearest}
    * Chromatic Point
    * Chromatic Line
* Convex Mask

Support for ConvexMask, will fail or flip triangle if the shape is not convex. In practice some non-convex shape could work if all vertices are visible from the corner of the BoundingBox Used.

![](https://media.githubusercontent.com/media/soufianekhiat/DearWidgetsImages/main/Images/kYA3Dw6TmH.gif)

* DrawColorDensityPlot (aka ShaderToy)

Use carefully that can have impact on your performances for HighRes canvas or/and expensive lambda.

![](https://media.githubusercontent.com/media/soufianekhiat/DearWidgetsImages/main/Images/us8Fc2jkIh.png)
![](https://media.githubusercontent.com/media/soufianekhiat/DearWidgetsImages/main/Images/yEGBSzv2F8.gif)

### Widgets
* Hue Selector

![](https://media.githubusercontent.com/media/soufianekhiat/DearWidgetsImages/main/Images/W0Q9VXNeGK.gif)


### Draft

Draft means draft.

#### Draw

* ChromaticityPlot

#### Widgets

* DragLengthScalar

![](https://media.githubusercontent.com/media/soufianekhiat/DearWidgetsImages/main/Images/XQ3kGD9aAW.gif)

* Slider 2D Float
A version for Slider2DScaler is available for (Im{S|U}{8,16,32,64}, Float and Double)

![](https://media.githubusercontent.com/media/soufianekhiat/DearWidgetsImages/main/Images/0dkkSCsb5Y.gif)

* Slider 2D Int

![](https://media.githubusercontent.com/media/soufianekhiat/DearWidgetsImages/main/Images/PGFHy3o6Tg.gif)

* Slider 3D

![](https://media.githubusercontent.com/media/soufianekhiat/DearWidgetsImages/main/Images/IQZMEeqfx0.gif)

* Grid

![](https://media.githubusercontent.com/media/soufianekhiat/DearWidgetsImages/main/Images/Wj5zT2ESJu.gif)

* 2D Move

![](https://media.githubusercontent.com/media/soufianekhiat/DearWidgetsImages/main/Images/FoeyB7aWSp.gif)

* Line Slider

![](https://media.githubusercontent.com/media/soufianekhiat/DearWidgetsImages/main/Images/4haBv2KuX7.gif)


## Constrains
C++ features used internally:
* std::string
* template
* constexpr
* if constexpr
* Lambda from template
* auto
