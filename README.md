# Rasterizer
A program that reads in an .obj triangle mesh to an image through software rasterization.

## What's Included
- Folder (_src_) with all the source code
- Folder (_resources_) with all the obj meshes tested

## Input / Output
As input, the rasterization program takes in six command arguments:
- OBJ file name
- Name of the image to output
- Desired image width
- Desired image height
- Mode for coloring (0, 1, or 2)
  - 0 for per-vertex colors
  - 1 for depth colors
  - 2 for y-axis gradient colors

For output, the program will create the image with the desired name (and format).

## Process

### Read in the triangles
Utilizing [Syoyo's Obj Loader](https://github.com/tinyobjloader/tinyobjloader), we first load the mesh into our program and store the vertex positions into a buffer, taking every three consecutive vertices and creating a single triangle from them.

### Convert the 3D coordinates into 2D image coordinates
With the triangles now forming a 3D object, we create a bounding box for every triangle and build another for the entirety of the object, allowing us to display the object in such a way where it fills as much of the screen as possible without distoriton.

### Draw triangles
Using our previously created bounding boxes, we utilize barycentric coordinates in order to change the shape of our bounding boxes into a triangle that matches the actual 3D object.

### Implement the Z-Buffer
Creating a buffer containing the z-coordinate of every pixel, we then store the interpolated z-coordinate of every vertex using the pixel's barycentric coordinates.

### Drawing the image
And with [Nothing's Image Writer](http://github.com/nothings/stb), we finally draw every triangle, pixel-by-pixel, into an output file.
