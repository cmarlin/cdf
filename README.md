Testing the concept of "Cascaded Distance Field"
The concept is an extension of distance field for texture alpha-cut but combined with many textures (maybe 4)
The goal is to create a kind of vector rendering.
Advantage
  * sharp edges
  * GPU compatible
  * Alpha-edge compatible (we can compute alpha coverage of pixel)
  * texture compression compatible
  * short shading
  * degrade compatible (based on texture internal color value)

Disavantage
  * Details limited to texture used as support. In fact it need twice the resolution of the details
    so, drawing musn't have tiny details. we needs custom or compatible SVG files
  * need many (up to 4 in theory) textures to draw any color pattern


Demo
http://cmarlin.github.io/cdf/index.html


