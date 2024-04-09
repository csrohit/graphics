# Lighting in OpenGL

<!--toc:start-->
- [Lighting in OpenGL](#lighting-in-opengl)
  - [How to run](#how-to-run)
    - [Compilation](#compilation)
    - [Execution](#execution)
  - [Possible Outcomes](#possible-outcomes)
  - [Description](#description)
    - [With Lighting](#with-lighting)
      - [Light0, BlueAmbientLight & texture disabled](#light0-blueambientlight-texture-disabled)
      - [Light0](#light0)
      - [BlueAmbient](#blueambient)
      - [Texture](#texture)
      - [BlueAmbient + Texture](#blueambient-texture)
      - [Light0 + Texture](#light0-texture)
      - [Light0 + BlueAmbientLight](#light0-blueambientlight)
      - [Light0 + BlueAmbientLight + Texture](#light0-blueambientlight-texture)
    - [Without Lighting](#without-lighting)
      - [Textured enabled](#textured-enabled)
      - [Texture disabled](#texture-disabled)
  - [Output](#output)
  - [Conclusion](#conclusion)
<!--toc:end-->

## How to run

### Compilation

The program can be compiled on windows by executing the `build.bat`.

### Execution

execute `ogl.exe` to run the program. Given below are the menu options which can be used to test various input conditions

- `t` - toggle texture
- `l` - toggle lighting
- `m` - toggle blue ambient light
- `0` - toggle Light0

## Possible Outcomes

| Lighting | Light0 | BlueAmbientLight | Texture | Final output                                                     | Description                                                   |
|----------|--------|------------------|---------|------------------------------------------------------------------|---------------------------------------------------------------|
| y        | y      | y                | y       | Faces towards light are Yellow others are Dark Blue with Texture | [Light+Texture+BlueAmbient](#light0-blueambientlight-texture) |
| y        | y      | y                | n       | Faces towards light are Yellow others are Dark Blue              | [Light+BlueAmbient](#light0-blueambientlight)                 |
| y        | y      | n                | y       | Faces towards light are Yellow others are Dark Grey with Texture | [Light+Texture](#light0-texture)                              |
| y        | y      | n                | n       | Faces towards light are Yellow others are Dark Grey              | [Light](#light0)                                              |
| y        | n      | y                | y       | Dark Blue with Texture                                           | [BlueAmbient+Texture](#blueambient-texture)                   |
| y        | n      | y                | n       | Dark Blue                                                        | [BlueAmbient](#blueambient-texture)                           |
| y        | n      | n                | y       | Dark Grey with Texture                                           | [Texture](#texture)                                           |
| y        | n      | n                | n       | Dark Grey                                                        | [Lighting+None](#light0-blueambientlight-texture-disabled)    |
| n        | y      | y                | y       | Textured                                                         | [No_Lighting+Texture](#without-lighting)                      |
| n        | y      | y                | n       | Red                                                              | [No_Lighting+None](#light0-blueambientlight-texture-disabled) |
| n        | y      | n                | y       | Textured                                                         | [No_Lighting+Texture](#without-lighting)                      |
| n        | y      | n                | n       | Red                                                              | [No_Lighting+None](#light0-blueambientlight-texture-disabled) |
| n        | n      | y                | y       | Textured                                                         | [No_Lighting+Texture](#without-lighting)                      |
| n        | n      | y                | n       | Red                                                              | [No_Lighting+None](#light0-blueambientlight-texture-disabled) |
| n        | n      | n                | y       | Textured                                                         | [No_Lighting+Texture](#without-lighting)                      |
| n        | n      | n                | n       | Red                                                              | [No_Lighting+None](#light0-blueambientlight-texture-disabled) |

## Description

### With Lighting

By default lighting is disabled in OpenGL and we have to explicitly enable it by calling `glEnable(GL_LIGHTING)`. You also need to explicitly enable each light source that you define, after you've specified the parameters for that source.

#### **Light0, BlueAmbientLight & texture disabled**

OpenGL has a global ambient light source defined by `GL_LIGHT_MODEL_AMBIENT`, this source only has ambient light with default value of ![#333333](https://placehold.co/15x15/333333/333333.png) (0.2, 0.2, 0.2, 1.0). This lights up all the geometries in the scene evenly, so we might expect our cube to be colored ![#333333](https://placehold.co/15x15/333333/333333.png) (0.2, 0.2, 0.2, 1.0) but this is not the case. When Lighting is enabled, the colors are scaled by the reflectance properties of material. The default ambient reflectance is ![#333333](https://placehold.co/15x15/333333/333333.png) (0.2, 0.2, 0.2, 0.2) so the final color comes out as follows:

    ```C
    globalAmbient   = (0.2, 0.2, 0.2, 1.0)              // color value
    materialAmbient = (0.2, 0.2, 0.2, 1.0)              // scaling value
    outColor        = (0.2 * 0.2, 0.2 * 0.2, 0.2 * 0.2, 1.0) // final scaled color
    outColor        = (0.04, 0.04, 0.04, 1.0)
    // alpha component at a vertex is equal to the material's diffuse alpha value at that vertex
    ```

    So in the absence of any light source, the world is lit by global background ambient light and after reflecting from the Cube our eyes (camera) sees the following color ![#0a0a0a](https://placehold.co/15x15/0a0a0a/0a0a0a.png) (0.04, 0.04, 0.04, 1.0).

#### **Light0**

Light0 is given Diffuse & Ambient Color ![#ffff00](https://placehold.co/15x15/ffff00/ffff00.png) (1.0, 1.0f, 1.0f, 1.0). Global light source does not have a Diffuse component so it does not contribute to Diffuxe Color reflected by the material.\
 OpenGL uses the assigned normal to determine how much light that particular vertex receives from each light source. The amount of light received by a vertex is maximum when it is facing directly towards the light i.e. Surface/Vertex normal is parallel to the direction of light.
*note: Simplicity we are not calculating Emission and Specular components.*
The colors for the Cube are calculated as follows:-

1. *Vertices directly facing the Light0*: Light0 will have its maximum contribution

    ```C
    lightAmbient    = (1.0, 1.0, 0.0, 1.0) // color value
    globalAmbient   = (0.2, 0.2, 0.2, 1.0) // color value
    materialAmbient = (0.2, 0.2, 0.2, 1.0) // scaling value
    outAmbient      = materialAmbient * (lightAmbient + globalAmbient)
    outAmbient      = (0.2 * (1.0 + 0.2), 0.2 * (1.0 + 0.2), 0.2 * (0.0 + 0.2), 1.0f) 
    outAmbient      = (0.024, 0.024, 0.04, 1.0)
    
    lightDiffused    = (1.0, 1.0, 0.0, 1.0) // color value
    materialDiffused = (0.8, 0.8, 0.8, 1.0) // scaling value
    outDiffused      = materialDiffused * lightDiffused
    outDiffused      = (0.8 * 1.0, 0.8 * 1.0, 0.8 * 0.0, 1.0f) 
    outDiffused      = (0.8, 0.8, 0.0, 1.0)
    
    outColor = outAmbient + outDiffused
    outColor = (0.824, 0.824, 0.04, 1.0)
    ```

2. *Vertices opposite to Light0*: Light0 will have no contribution

    ```C
    globalAmbient   = (0.2, 0.2, 0.2, 1.0)              // color value
    materialAmbient = (0.2, 0.2, 0.2, 1.0)              // scaling value
    outColor        = (0.2*0.2, 0.2*0.2, 0.2*0.2, 1.0) // final scaled color
    outColor        = (0.04, 0.04, 0.04, 1.0)
    ```

3. *Vertices not directly facing nor opposite to Light0*: Light0 will contribute depending on the surface/vertext normal

#### **BlueAmbient**

This mode is similar to [Light0, BlueAmbientLight & texture disabled](#light0-blueambientlight-texture-disabled). The only difference being the `globalAmbient` color changed to ![#0000ff](https://placehold.co/15x15/0000ff/0000ff.png) `(0.0, 0.0, 1.0, 1.0)`.\
The color calculations are as follows:

```C
globalAmbient   = (0.0, 0.0, 1.0, 1.0)              // color value
materialAmbient = (0.2, 0.2, 0.2, 1.0)              // scaling value
outColor        = (0.2*0.0, 0.2*0.0, 0.2*1.0, 1.0) // final scaled color
outColor        = (0.0, 0.0, 0.2, 1.0)
```

So the world is lit by ![#0000ff](https://placehold.co/15x15/0000ff/0000ff.png) `(0.0, 0.0, 1.0, 1.0)` ambient light and after reflecting from the Cube our eyes (camera) sees the following color ![#0a0a0a](https://placehold.co/15x15/000033/000033.png) (0.00, 0.00, 0.2, 1.0).

#### **Texture**

This mode is similar to [Without Lighting](#without-lighting), the only difference is that the color from texture is scaled by `diffuseReflection` of material and `outColor` from scaled `globalAmbient` color is added to it.
The color calculations are as follows;

```C
outAmbient = (textureColor + globalAmbient) * materialAmbient
outDiffuse = textureColor * materialDiffuse
outColor = outAmbient + outDiffuse
```

#### **BlueAmbient + Texture**

This mode is a combination of two modes i.e. [BlueAmbient](#blueambient) & [Texture](#texture). The only difference is that the `globalAmbient` color is changed from ![#333333](https://placehold.co/15x15/333333/333333.png) `(0.2, 0.2, 0.2, 1.0)` to ![#0000ff](https://placehold.co/15x15/0000ff/0000ff.png) `(0.0, 0.0, 1.0, 1.0)`.
The color calculations are exactly same as prev mode.

```C
outAmbient = (textureColor + globalAmbient) * materialAmbient
outDiffuse = textureColor * materialDiffuse
outColor = outAmbient + outDiffuse
```

#### **Light0 + Texture**

This mode is a combination of two modes i.e. [Light0](#light0) & [Texture](#texture). Color from texture is added to `diffuseColor` as all vertices.
Color calculations are as follows:

1. *Vertices directly facing the Light0*: Light0 will have its maximum contribution, texture will contribute to both ambient & diffuse color

```C
outAmbient  = materialAmbient * (lightAmbient + globalAmbient + textureColor)
outDiffused = materialDiffused * (lightDiffused + textureColor)

outColor = outAmbient + outDiffused
```

2. *Vertices opposite to Light0*: Light0 will have no contribution, texture will contribute to both ambient & diffuse color

```C
outAmbient  = materialAmbient * (globalAmbient + textureColor)
outDiffused = materialDiffused * textureColor

outColor = outAmbient + outDiffused
```

3. *Vertices not directly facing nor opposite to Light0*: Light0 will contribute depending on the surface/vertext normal and texture will contribute to both ambient & diffuse color

#### **Light0 + BlueAmbientLight**

This mode is same as [Light0](#light0) with the only change being the `globalAmbient` color which is changed from ![#333333](https://placehold.co/15x15/333333/333333.png) `(0.2, 0.2, 0.2, 1.0)` to ![#0000ff](https://placehold.co/15x15/0000ff/0000ff.png) `(0.0, 0.0, 1.0, 1.0)`.
Color calculations are as follows:

1. *Vertices directly facing the Light0*: Light0 will have its maximum contribution for both ambient & diffuse while `globalAmbient` will evenly contribute to ambient component

```C
outAmbient  = materialAmbient * (lightAmbient + globalAmbient)
outDiffused = materialDiffused * lightDiffused

outColor = outAmbient + outDiffused
```

2. *Vertices opposite to Light0*: Light0 will have no contribution while `globalAmbient` will evenly contribute to ambient component

```C
outAmbient  = materialAmbient * globalAmbient
outDiffused = 0

outColor = outAmbient + outDiffused
```

3. *Vertices not directly facing nor opposite to Light0*: Light0 will contribute depending on the surface/vertext normal and texture will contribute to both ambient & diffuse col

#### **Light0 + BlueAmbientLight + Texture**

This mode is same as [Light0 + Texture](#light0-texture) with the only change being the `globalAmbient` color which is changed from ![#333333](https://placehold.co/15x15/333333/333333.png) `(0.2, 0.2, 0.2, 1.0)` to ![#0000ff](https://placehold.co/15x15/0000ff/0000ff.png) `(0.0, 0.0, 1.0, 1.0)`.
Color calculations are as follows:

1. *Vertices directly facing the Light0*: Light0 will have its maximum contribution, texture will contribute to both ambient & diffuse color and `globalAmbient` will contribute evenly to ambient color

```C
outAmbient  = materialAmbient * (lightAmbient + globalAmbient + textureColor)
outDiffused = materialDiffused * (lightDiffused + textureColor)

outColor = outAmbient + outDiffused
```

2. *Vertices opposite to Light0*: Light0 will have no contribution, texture will contribute to both ambient & diffuse colorr and `globalAmbient` will contribute evenly to ambient color

```C
outAmbient  = materialAmbient * (globalAmbient + textureColor)
outDiffused = materialDiffused * textureColor

outColor = outAmbient + outDiffused
```

3. *Vertices not directly facing nor opposite to Light0*: Light0 will contribute depending on the surface/vertext normal and texture will contribute to both ambient & diffuse colorr and `globalAmbient` will contribute evenly to ambient color

### Without Lighting

If lighting isn't enabled, the current color is simply mapped onto the current vertex, and no calculations concerning normals, light sources, the lighting model, and material properties are performed.
So in this case Light0 and BlueAmbientLight will have no effect on the final output.

#### **Textured enabled**

 will appear textured once textured is enabled else we will observe the default output i.e. Red colored cube.

#### **Texture disabled**

In this case the final output color will be the color given to the vertex by call to `glColor3f(...)` i.e. ![#f03c15](https://placehold.co/15x15/ff0000/ff0000.png) `(1.0, 0.0, 0.0, 1.0)`

## Output

Click on the link to view the video demo for lighting. Given below are the transitions used in the video.\
[![Light description](https://img.youtube.com/vi/bhTMx7mM0pI/0.jpg)](https://www.youtube.com/watch?v=bhTMx7mM0pI)

[Default: Lighting is off, no Texture, glColor is red]
Transitions

1. Lighting is turned on -- nearly black output
2. Lighting is turned off -- normal red output
3. Enable Texture  -- o/p with Texture
4. Disable Texture -- Red output
4. Lighting is turned on -- nearly black output
5. Ambient Lighting on -- dark blue output
6. Ambient Lighting off -- nearly black output

7. Light0 turned  on -- yellow output
8. Light 0 turned off -- nearly black output

9. Enable Texture  -- nearly black o/p with Texture
10. Disable Texture -- nearly black output

11. Light0 turned  on -- yellow output
12. Enable Texture  -- yellow o/p with Texture
13. Light 0 turned off -- nearly black o/p with Texture
14. Ambient  Lighting on -- dark blue o/p with Texture

15. Light0 turned  on -- faces facing light are yellow and other are dark blue with Texture
16. Texture Turned off-- faces facing light are yellow and other are dark blue

## Conclusion


