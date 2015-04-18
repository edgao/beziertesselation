#include <float.h>
#include "Class.h"
#include <iostream>
#include <cstdio>

void append(vector<Triangle*>* a, vector<Triangle*>* b) {
	for (int i = 0; i < b->size(); i++) {
		a->push_back((*b)[i]);
	}
}

BezierCurve::BezierCurve() {}
BezierCurve::BezierCurve(Vector3f p[]) {
	for (int i = 0; i < 4; i++) {
		points[i] = p[i];
	}
}
Vector3f BezierCurve::evaluate(float t) {
	return pow(1 - t, 3) * points[0] + 3 * pow(1 - t, 2) * t * points[1] + 3 * (1 - t) * pow(t, 2) * points[2] + pow(t, 3) * points[3];
}

BezierPatch::BezierPatch() {}
BezierPatch::BezierPatch(BezierCurve c[]) {
	for (int i = 0; i < 4; i++) {
		curves[i] = c[i];
	}
}
Vector3f BezierPatch::evaluate(Vector2f u) {
	Vector3f p[4];
	for (int i = 0; i < 4; i++) {
		p[i] = curves[i].evaluate(u(0));
	}
	return (BezierCurve(p)).evaluate(u(1));
}

Triangle::Triangle() {}
Triangle::Triangle(Vector3f a, Vector3f b, Vector3f c, Vector3f n1, Vector3f n2, Vector3f n3) {
	vertices[0] = a;
	vertices[1] = b;
	vertices[2] = c;
	normals[0] = n1;
	normals[1] = n2;
	normals[2] = n3;
}
Triangle::Triangle(Vector3f v[], Vector3f n[]) {
	for (int i = 0; i < 3; i++) {
		vertices[i] = v[i];
		normals[i] = n[i];
	}
}
Vector3f Triangle::getFlatNormal() {
	return (normals[0] + normals[1] + normals[2]) / 3.0;
}

BezierPatchTesselator::BezierPatchTesselator() {}
BezierPatchTesselator::BezierPatchTesselator(vector<BezierPatch*>* p) {
	patches = *p;
}
bool BezierPatchTesselator::isFlat(BezierPatch p, Vector3f u, Vector3f ref, float threshold) {
	return (u - ref).norm() < threshold;
}
vector<Triangle*>* BezierPatchTesselator::tesselate(int mode, bool center_test, float threshold) {
	if (mode == UNIFORM_MODE) {
		vector<Triangle*>* triangles = new vector<Triangle*>();

		for (int patchNum = 0; patchNum < patches.size(); patchNum++) {
			BezierPatch p = *patches[patchNum];
			float u = 0;
			while (u < 1 - threshold) {
				float v = 0;
				while (v < 1 - threshold) {
					Vector2f a, b, c;
					a = (Vector2f() << u, v).finished();
					b = (Vector2f() << u + threshold, v).finished();
					c = (Vector2f() << u, v + threshold).finished();
					triangles->push_back(new Triangle(p.evaluate(a), p.evaluate(b), p.evaluate(c), p.findNormal(a), p.findNormal(b), p.findNormal(c)));

					a = (Vector2f() << u + threshold, v + threshold).finished();
					b = (Vector2f() << u + threshold, v).finished();
					c = (Vector2f() << u, v + threshold).finished();
					triangles->push_back(new Triangle(p.evaluate(a), p.evaluate(b), p.evaluate(c), p.findNormal(a), p.findNormal(b), p.findNormal(c)));
					v += threshold;
				}
				u += threshold;
			}
		}
		return triangles;
	}

	vector<Triangle*>* triangles = new vector<Triangle*>();
	for (int patchNum = 0; patchNum < patches.size(); patchNum++) {
		Vector2f vertices[3];
		vertices[0] = (Vector2f() << 0, 0).finished();
		vertices[1] = (Vector2f() << 0, 1).finished();
		vertices[2] = (Vector2f() << 1, 0).finished();
		vector<Triangle*>* temp = tesselateTriangle(mode, center_test, vertices, *(patches[patchNum]), threshold);
		append(triangles, temp);

		vertices[0] = (Vector2f() << 1, 1).finished();
		vertices[1] = (Vector2f() << 0, 1).finished();
		vertices[2] = (Vector2f() << 1, 0).finished();
		temp = tesselateTriangle(mode, center_test, vertices, *(patches[patchNum]), threshold);
		append(triangles, temp);
	}
	return triangles;
}

vector<Triangle*>* BezierPatchTesselator::tesselateTriangle(int mode, bool center_test, Vector2f vertices[], BezierPatch p, float threshold) {
	// If this triangle is too small, return
	if ((vertices[0] - vertices[1]).norm() < threshold) {
		vector<Triangle*>* ret = new vector<Triangle*>();
		Vector3f norm = p.evaluate(vertices[0]).cross(p.evaluate(vertices[1])).normalized();
		ret->push_back(new Triangle(p.evaluate(vertices[0]), p.evaluate(vertices[1]), p.evaluate(vertices[2]), p.findNormal(vertices[0]), p.findNormal(vertices[1]), p.findNormal(vertices[2])));
		return ret;
	}
	vector<Triangle*>* ret = new vector<Triangle*>();
	if (center_test) {
		Vector2f centerPoint = (vertices[0] + vertices[1] + vertices[2]) / 3;
		bool center_flat = isFlat(p, (p.evaluate(vertices[0]) + p.evaluate(vertices[1]) + p.evaluate(vertices[2])) / 3, p.evaluate(centerPoint), threshold);
		if (!center_flat) {
			// The center of this triangle isn't flat
			for (int side = 0; side < 3; side++) {
				Vector2f a = vertices[side], b = vertices[(side + 1) % 3];
				Vector2f midpoint = (a + b) / 2;
				Vector3f ref = p.evaluate(midpoint);
				if (isFlat(p, (p.evaluate(a) + p.evaluate(b)) / 2, ref, threshold)) {
					// If this side of the triangle is flat, make it a polygon
					Vector3f norm = p.evaluate(a).cross(p.evaluate(b));
					ret->push_back(new Triangle(p.evaluate(a), p.evaluate(b), p.evaluate(centerPoint), p.findNormal(a), p.findNormal(b), p.findNormal(centerPoint)));
				} else {
					// Otherwise, subdivide and recurse
					Vector2f newVertices[3];
					newVertices[0] = a;
					newVertices[1] = midpoint;
					newVertices[2] = centerPoint;
					append(ret, tesselateTriangle(mode, center_test, newVertices, p, threshold));

					newVertices[0] = b;
					newVertices[1] = midpoint;
					newVertices[2] = centerPoint;
					append(ret, tesselateTriangle(mode, center_test, newVertices, p, threshold));
				}
			}
			return ret;
		}
	}
	// Either we're not doing center_test, or the center point is flat anyway
	bool nonflat[3];
	int num = 0;
	int sides[3];
	for (int side = 0; side < 3; side++) {
		Vector2f a = vertices[side], b = vertices[(side + 1) % 3];
		Vector2f midpoint = (a + b) / 2;
		Vector3f ref = p.evaluate(midpoint);
		nonflat[side] = !isFlat(p, (p.evaluate(a) + p.evaluate(b)) / 2, ref, threshold);
		if (nonflat[side]) {
			sides[num] = side;
			num++;
		}
	}
	if (num == 3) {
		// All sides are nonflat, subdivide into 4 triangles
		// Handle the three corners
		for (int side = 0; side < 3; side++) {
			Vector2f newVertices[3];
			newVertices[0] = vertices[side];
			newVertices[1] = (vertices[(side + 1) % 3] + vertices[side]) / 2;
			newVertices[2] = (vertices[(side + 2) % 3] + vertices[side]) / 2;
			// cout << "ASDF\n" << vertices[side] << endl << vertices[(side + 1) % 3] << endl << vertices[(side + 2) % 3] << endl
			// 	<< newVertices[0] << endl << newVertices[1] ;
			vector<Triangle*>* temp = tesselateTriangle(mode, center_test, newVertices, p, threshold);
			append(ret, temp);
			free(temp);
		}
		// Handle the middle triangle
		Vector2f newVertices[3];
		for (int side = 0; side < 3; side++) {
			newVertices[side] = (vertices[side] + vertices[(side + 1) % 3]) / 2;
		}
		vector<Triangle*>* temp = tesselateTriangle(mode, center_test, newVertices, p, threshold);
		append(ret, temp);
		free(temp);
	} else if (num >= 1) {
		if (num == 2) {
			// One side is flat; 3 sub-triangles 
			Vector2f a, b, centerPoint, midpoint;
			a = vertices[sides[0]];
			b = vertices[(sides[0] + 1) % 3];
			centerPoint = vertices[(sides[0] + 2) % 3];
			midpoint = (a + b) / 2;

			Vector2f newVertices[3];
			newVertices[0] = a;
			newVertices[1] = midpoint;
			newVertices[2] = centerPoint;
			vector<Triangle*>* temp = tesselateTriangle(mode, center_test, newVertices, p, threshold);
			append(ret, temp);
			free(temp);

			a = (b + centerPoint) / 2;
			newVertices[0] = b;
			newVertices[1] = midpoint;
			newVertices[2] = a;
			temp = tesselateTriangle(mode, center_test, newVertices, p, threshold);
			append(ret, temp);
			free(temp);

			newVertices[0] = a;
			newVertices[1] = midpoint;
			newVertices[2] = centerPoint;
			temp = tesselateTriangle(mode, center_test, newVertices, p, threshold);
			append(ret, temp);
			free(temp);
		} else {
			// Two sides are flat; 2 sub-triangles
			Vector2f a, b, centerPoint, midpoint;
			a = vertices[sides[0]];
			b = vertices[(sides[0] + 1) % 3];
			centerPoint = vertices[(sides[0] + 2) % 3];
			midpoint = (a + b) / 2;

			Vector2f newVertices[3];
			newVertices[0] = a;
			newVertices[1] = midpoint;
			newVertices[2] = centerPoint;
			vector<Triangle*>* temp = tesselateTriangle(mode, center_test, newVertices, p, threshold);
			append(ret, temp);
			free(temp);

			newVertices[0] = b;
			newVertices[1] = midpoint;
			newVertices[2] = centerPoint;
			temp = tesselateTriangle(mode, center_test, newVertices, p, threshold);
			append(ret, temp);
			free(temp);
		}
	} else {
		// All sides are flat; this triangle is a good polygon
		Vector3f norm = p.evaluate(vertices[2]).cross(p.evaluate(vertices[1])).normalized();
		ret->push_back(new Triangle(p.evaluate(vertices[0]), p.evaluate(vertices[1]), p.evaluate(vertices[2]), p.findNormal(vertices[0]), p.findNormal(vertices[1]), p.findNormal(vertices[2])));
	}
	return ret;
}

Vector3f BezierPatch::findNormal(Vector2f uv){
  // returns a normal vector to the point
  Vector3f ret;
  float u = uv(0);
  float v = uv(1);
  for (int i=0; i<4; i++){
    for (int j=0; j<4; j++){
      if (i == 0 and j == 0){
        ret += pow(1 - u, 3) * pow(1 - v, 3) * this->evaluate(uv);
      }else if (i == 0 and j == 1){
        ret += pow(1 - u, 3) * 3*v*pow(1-v, 2) * this->evaluate(uv);
      }else if (i == 0 and j == 2){
        ret += pow(1 - u, 3) * 3*pow(v, 2)*(1-v) * this->evaluate(uv);
      }else if (i == 0 and j == 3){
        ret += pow(1 - u, 3) * pow(v, 3) * this->evaluate(uv);
      }else if (i == 1 and j == 0){
        ret += 3*u*pow(1 - u, 2) * pow(1 - v, 3) * this->evaluate(uv);
      }else if (i == 1 and j == 1){
        ret += 3*u*pow(1 - u, 2) * 3*v*pow(1-v, 2) * this->evaluate(uv);
      }else if (i == 1 and j == 2){
        ret += 3*u*pow(1 - u, 2) * 3*pow(v, 2)*(1-v) * this->evaluate(uv);
      }else if (i == 1 and j == 3){
        ret += 3*u*pow(1 - u, 2) * pow(v, 3) * this->evaluate(uv);
      }else if (i == 2 and j == 0){
        ret += 3*pow(u,2)*(1 - u) * pow(1 - v, 3) * this->evaluate(uv);
      }else if (i == 2 and j == 1){
        ret += 3*pow(u,2)*(1 - u) * 3*v*pow(1-v, 2) * this->evaluate(uv);
      }else if (i == 2 and j == 2){
        ret += 3*pow(u,2)*(1 - u) * 3*pow(v, 2)*(1-v) * this->evaluate(uv);
      }else if (i == 2 and j == 3){
        ret += 3*pow(u,2)*(1 - u) * pow(v, 3) * this->evaluate(uv);
      }else if (i == 3 and j == 0){
        ret += pow(u, 3) * pow(1 - v, 3) * this->evaluate(uv);
      }else if (i == 3 and j == 1){
        ret += pow(u, 3) * 3*v*pow(1-v, 2) * this->evaluate(uv);
      }else if (i == 3 and j == 2){
        ret += pow(u, 3) * 3*pow(v, 2)*(1-v) * this->evaluate(uv);
      }else if (i == 3 and j == 3){
        ret += pow(u, 3) * pow(v, 3) * this->evaluate(uv);
      }
    }
  }
  return ret;
}    
