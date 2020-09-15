#include <iostream>
#include <string>
#include <algorithm>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "Image.h"

// This allows you to skip the `std::` in front of C++ standard library
// functions. You can also say `using std::cout` to be more selective.
// You should never do this in a header file.
using namespace std;

class Vertex { //A vertex that contains the x, y, z coordinates and a random color determined upon creation.
public:
	float xCor;
	float yCor;
	float zCor;
	unsigned char vColor = rand() % 256;
};

class Triangle { //Contains three vertices.
public:
	Vertex* vert1;
	Vertex* vert2;
	Vertex* vert3;
};

void createTriangles(vector<float> &posBuf, vector<Vertex*> &vertices, vector<Triangle*> &triangles) {
	//Throws all x, y, and z coordinates into their own vertices.
	for (int i = 0; i < posBuf.size() - 1; i++) {
		Vertex* newVertex = new Vertex;
		newVertex->xCor = posBuf[i];
		i++;
		newVertex->yCor = posBuf[i];
		i++;
		newVertex->zCor = posBuf[i];
		vertices.push_back(newVertex);
	}

	//Throws all of those recently-created vertices into their own triangles.
	for (int i = 0; i < vertices.size() - 1; i++) {
		Triangle* newTriangle = new Triangle;
		newTriangle->vert1 = vertices[i];
		i++;
		newTriangle->vert2 = vertices[i];
		i++;
		newTriangle->vert3 = vertices[i];
		triangles.push_back(newTriangle);
	}
}

void createScaler(vector<Triangle*> &triangles , float &xTrans, float &xScale, float &yTrans, float &yScale, float &zTrans, float &zScale, float iW, float iH, vector<float>& yBuf) {
	float minX = triangles[0]->vert1->xCor, maxX = triangles[0]->vert1->xCor;
	float minY = triangles[0]->vert1->yCor, maxY = triangles[0]->vert1->yCor;
	float minZ = triangles[0]->vert1->zCor, maxZ = triangles[0]->vert1->zCor;

	//Implementing the scaling technqiues discussed in class to fill the output image. Stores values, doesn't use them yet.
	for (int i = 0; i < triangles.size(); ++i) {
		minX = std::min({ minX, triangles[i]->vert1->xCor, triangles[i]->vert2->xCor, triangles[i]->vert3->xCor });
		maxX = std::max({ maxX, triangles[i]->vert1->xCor, triangles[i]->vert2->xCor, triangles[i]->vert3->xCor });
		minY = std::min({ minY, triangles[i]->vert1->yCor, triangles[i]->vert2->yCor, triangles[i]->vert3->yCor });
		maxY = std::max({ maxY, triangles[i]->vert1->yCor, triangles[i]->vert2->yCor, triangles[i]->vert3->yCor });
		minZ = std::min({ minZ, triangles[i]->vert1->zCor, triangles[i]->vert2->zCor, triangles[i]->vert3->zCor });
		maxZ = std::max({ maxZ, triangles[i]->vert1->zCor, triangles[i]->vert2->zCor, triangles[i]->vert3->zCor });
	}

	yBuf.push_back(minY);
	yBuf.push_back(maxY);

	xTrans = -minX;
	yTrans = -minY;
	zTrans = -minZ;
	if (iW < iH) {
		xScale = iW / (maxX - minX);
		yScale = iW / (maxY - minY);
		yTrans += (iH / yScale / 2) - ((maxY - minY) / 2);
	} else if (iH < iW) {
		xScale = iH / (maxY - minY);
		yScale = iH / (maxY - minY);
		xTrans += (iW / xScale / 2) - ((maxX - minX) / 2);
	} else if (iW == iH) {
		xScale = iW / (maxX - minX);
		yScale = iH / (maxY - minY);
	}
	zScale = (maxZ - minZ);
}

float coordinateScaler(float coordinate, float translation, float scale) {
	//Just simplifies the act of scaling a coordinate.
	return ((coordinate + translation) * scale);
}

void triangleScaler(Triangle*& triangle, float xTranslation, float xScale, float yTranslation, float yScale, float zTranslation, float zScale) {
	//Scales all the x, y, and z coordinates for every triangle.
	triangle->vert1->xCor = coordinateScaler(triangle->vert1->xCor, xTranslation, xScale);
	triangle->vert1->yCor = coordinateScaler(triangle->vert1->yCor, yTranslation, yScale);
	triangle->vert2->xCor = coordinateScaler(triangle->vert2->xCor, xTranslation, xScale);
	triangle->vert2->yCor = coordinateScaler(triangle->vert2->yCor, yTranslation, yScale);
	triangle->vert3->xCor = coordinateScaler(triangle->vert3->xCor, xTranslation, xScale);
	triangle->vert3->yCor = coordinateScaler(triangle->vert3->yCor, yTranslation, yScale);
	triangle->vert1->zCor = ((triangle->vert1->zCor + zTranslation) / zScale) * 255;
	triangle->vert2->zCor = ((triangle->vert2->zCor + zTranslation) / zScale) * 255;
	triangle->vert3->zCor = ((triangle->vert3->zCor + zTranslation) / zScale) * 255;
}

bool isInside(float Px, float Py, Triangle* &triangle, float& alpha, float& beta, float& gamma) {
	//Checks whether or not the given x and y coordinates shoudl be drawn.
	float topAlpha = (triangle->vert2->yCor - triangle->vert3->yCor) * (Px - triangle->vert3->xCor);
	topAlpha += (triangle->vert3->xCor - triangle->vert2->xCor) * (Py - triangle->vert3->yCor);

	float topBeta = (triangle->vert3->yCor - triangle->vert1->yCor) * (Px - triangle->vert3->xCor);
	topBeta += (triangle->vert1->xCor - triangle->vert3->xCor) * (Py - triangle->vert3->yCor);

	float area = (triangle->vert2->yCor - triangle->vert3->yCor) * (triangle->vert1->xCor - triangle->vert3->xCor);
	area += (triangle->vert3->xCor - triangle->vert2->xCor) * (triangle->vert1->yCor - triangle->vert3->yCor);

	alpha = topAlpha / area;
	beta = topBeta / area;
	gamma = 1 - alpha - beta;

	if (gamma >= -0.04 && beta >= -0.04 && alpha >= -0.04) {
		return 1;
	}
	else {
		return 0;
	}
}

void populateZBuffer(vector<vector<float>>& zBuf, int imageWidth, int imageHeight) {
	//Goes through every pixel, assigns it a z value of -1e8, and pushes it into the ZBuffer.
	for (int i = 0; i <= imageWidth; i++) {
		vector<float> newVector;
		for (int j = 0; j <= imageHeight; j++) {
			newVector.push_back(-1e8);
		}
		zBuf.push_back(newVector);
	}
}

float ZCheck(float Px, float Py, Triangle * &triangle) {
	//Calculates the z of a specific point.
	float z3x1y2 = triangle->vert3->zCor * (Px - triangle->vert1->xCor) * (Py - triangle->vert2->yCor);
	float z1x2y3 = triangle->vert1->zCor * (Px - triangle->vert2->xCor) * (Py - triangle->vert3->yCor);
	float z2x3y1 = triangle->vert2->zCor * (Px - triangle->vert3->xCor) * (Py - triangle->vert1->yCor);
	float z2x1y3 = triangle->vert2->zCor * (Px - triangle->vert1->xCor) * (Py - triangle->vert3->yCor);
	float z3x2y1 = triangle->vert3->zCor * (Px - triangle->vert2->xCor) * (Py - triangle->vert1->yCor);
	float z1x3y2 = triangle->vert1->zCor * (Px - triangle->vert3->xCor) * (Py - triangle->vert2->yCor);

	float x1y2 = (Px - triangle->vert1->xCor) * (Py - triangle->vert2->yCor);
	float x2y3 = (Px - triangle->vert2->xCor) * (Py - triangle->vert3->yCor);
	float x3y1 = (Px - triangle->vert3->xCor) * (Py - triangle->vert1->yCor);
	float x1y3 = (Px - triangle->vert1->xCor) * (Py - triangle->vert3->yCor);
	float x2y1 = (Px - triangle->vert2->xCor) * (Py - triangle->vert1->yCor);
	float x3y2 = (Px - triangle->vert3->xCor) * (Py - triangle->vert2->yCor);

	float z = (z3x1y2 + z1x2y3 + z2x3y1 - z2x1y3 - z3x2y1 - z1x3y2) / (x1y2 + x2y3 + x3y1 - x1y3 - x2y1 - x3y2);

	return z;
}

void DrawPixels(float Px, float Py, vector<float> yBuf, vector<vector<float>>& zBuf, Triangle * &triangle, shared_ptr<Image> image, int colorMode) {
	//Draws the pixel depending on the color mode.
	
	float alpha = 0;
	float beta = 0;
	float gamma = 0;

	if (colorMode == 0) {
		if (isInside(Px, Py, triangle, alpha, beta, gamma)) {
			unsigned char r = (alpha +.04) * triangle->vert1->vColor;
			unsigned char g = (beta + .04) * triangle->vert2->vColor;
			unsigned char b = (gamma + .04) * triangle->vert3->vColor;
			image->setPixel(Px, Py, r, g, b);
		}
	} else if (colorMode == 1) {
		if (isInside(Px, Py, triangle, alpha, beta, gamma)) {
			if (zBuf[Px][Py] < ZCheck(Px, Py, triangle)) {
				zBuf[Px][Py] = ZCheck(Px, Py, triangle);
			}
			unsigned char r = zBuf[Px][Py];
			image->setPixel(Px, Py, r, 0, 0);
		}
	} else if (colorMode == 2) {
		if (isInside(Px, Py, triangle, alpha, beta, gamma)) {
			unsigned char r = Py * (yBuf[1] - yBuf[0]);
			unsigned char g = 0;
			unsigned char b = 255 - Py * (yBuf[1] - yBuf[0]);
			image->setPixel(Px, Py, r, g, b);
		}
	}
}

int main(int argc, char **argv)
{
	if(argc < 6) {
		cout << "Usage: A1 meshfile filename width height color" << endl;
		return 0;
	}
	string meshName(argv[1]);
	string outputFilename(argv[2]);
	int imageWidth(atoi(argv[3]));
	int imageHeight(atoi(argv[4]));
	unsigned char colorMode(atoi(argv[5]));
	if (colorMode > 2) {
		cout << "Invalid Color Mode!" << endl;
		return 0;
	}

	// Load geometry
	vector<float> posBuf; // list of vertex positions
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	string errStr;
	bool rc = tinyobj::LoadObj(&attrib, &shapes, &materials, &errStr, meshName.c_str());
	if(!rc) {
		cerr << errStr << endl;
	} else {
		// Some OBJ files have different indices for vertex positions, normals,
		// and texture coordinates. For example, a cube corner vertex may have
		// three different normals. Here, we are going to duplicate all such
		// vertices.
		// Loop over shapes
		for(size_t s = 0; s < shapes.size(); s++) {
			// Loop over triangles (polygons)
			size_t index_offset = 0;
			for(size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
				size_t fv = shapes[s].mesh.num_face_vertices[f];
				// Loop over vertices in the triangle.
				for(size_t v = 0; v < fv; v++) {
					// access to vertex
					tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
					posBuf.push_back(attrib.vertices[3*idx.vertex_index+0]);
					posBuf.push_back(attrib.vertices[3*idx.vertex_index+1]);
					posBuf.push_back(attrib.vertices[3*idx.vertex_index+2]);
				}
				index_offset += fv;
				// per-triangle material (IGNORE)
				shapes[s].mesh.material_ids[f];
			}
		}
	}

	//Create a vector to store vertices, alongside a vector of triangles that contain the vertices.
	vector<Vertex *> vertices;
	vector<Triangle *> triangles;
	//The Zbuffer and YBuffer are created here to help with ColorMode 1 and 2.
	vector<vector<float>> zBuf;
	vector<float> yBuf;

	//Goes through every pixel and assigns it a Z-value of -1e8, storing it in the ZBuffer.
	populateZBuffer(zBuf, imageWidth, imageHeight);

	//Utilizes the coordinates and vertices to create a triangle class and pushing it unto its respective vector.
	createTriangles(posBuf, vertices, triangles);

	auto image = make_shared<Image>(imageWidth, imageHeight);

	//Initializes the Translation and Scale variables for the x, y, and z coordinates. These will be used VERY often.
	float xTrans = 0, yTrans = 0, zTrans = 0;
	float xScale = 1, yScale = 1, zScale = 1;

	//Creates the scaling that'll be required, given the image width and height alongside our current data.
	createScaler(triangles, xTrans, xScale, yTrans, yScale, zTrans, zScale, imageWidth, imageHeight, yBuf);

	//Goes through every triangle.
	for (int i = 0; i < triangles.size(); ++i) {
		
		//Scales all the coordinates.
		triangleScaler(triangles[i], xTrans, xScale, yTrans, yScale, zTrans, zScale);

		//Creates a box boundary around the triangle, allowing us to iterate through the pixels.
		float minX = std::min({ triangles[i]->vert1->xCor, triangles[i]->vert2->xCor, triangles[i]->vert3->xCor });
		float maxX = std::max({ triangles[i]->vert1->xCor, triangles[i]->vert2->xCor, triangles[i]->vert3->xCor });
		float minY = std::min({ triangles[i]->vert1->yCor, triangles[i]->vert2->yCor, triangles[i]->vert3->yCor });
		float maxY = std::max({ triangles[i]->vert1->yCor, triangles[i]->vert2->yCor, triangles[i]->vert3->yCor });
		
		//Goes through every pixel and draws it (or doesn't).
		for (float x = minX; x <= maxX; ++x) {
			for (float y = minY; y <= maxY; ++y) {
				DrawPixels(x, y, yBuf, zBuf, triangles[i], image, colorMode);
			}
		}
	}

	image->writeToFile(outputFilename);
	
	return 0;
}
