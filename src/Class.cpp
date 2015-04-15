#include <float.h>
#include "Class.h"
#include <iostream>
#include <cstdio>

void append(vector<Triangle*>* a, vector<Triangle*>* b) {
	return a->insert(a->end(), b->begin(), b->end());
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
Triangle::Triangle(Vector3f a, Vector3f b, Vector3f c) {
	vertices[0] = a;
	vertices[1] = b;
	vertices[2] = c;
}
Triangle::Triangle(Vector3f v[]) {
	for (int i = 0; i < 3; i++) {
		vertices[i] = v[i];
	}
}

BezierPatchTesselator::BezierPatchTesselator() {}
BezierPatchTesselator::BezierPatchTesselator(vector<BezierPatch*>* p) {
	patches = *p;
}
bool BezierPatchTesselator::isFlat(BezierPatch p, Vector2f u, Vector3f ref, float threshold) {
	Vector3f surf = p.evaluate(u);
	return (surf - ref).norm() < threshold;
}
vector<Triangle*>* BezierPatchTesselator::tesselate(int mode, bool center_test, float threshold) {
	vector<Triangle*>* triangles = (vector<Triangle*>*) malloc(sizeof(vector<Triangle*>));
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
	vector<Triangle*>* ret = (vector<Triangle*>*) malloc(sizeof(vector<Triangle*>));
	if (center_test) {
		Vector2f centerPoint = (vertices[0] + vertices[1] + vertices[2]) / 3;
		bool center_flat = isFlat(p, centerPoint, p.evaluate(centerPoint), threshold);
		if (!center_flat) {
			// The center of this triangle isn't flat
			for (int side = 0; side < 3; side++) {
				Vector2f a = vertices[side], b = vertices[(side + 1) % 3];
				Vector2f midpoint = (a + b) / 2;
				Vector3f ref = p.evaluate(midpoint);
				if (isFlat(p, midpoint, ref, threshold)) {
					// If this side of the triangle is flat, make it a polygon
					ret->push_back(new Triangle(p.evaluate(a), p.evaluate(b), p.evaluate(centerPoint)));
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
		nonflat[side] = isFlat(p, midpoint, ref, threshold);
		if (nonflat[side]) {
			sides[num] = side;
			num++;
		}
	}
	if (nonflat[2]) {
		// All sides are nonflat, subdivide into 4 triangles
	} else if (nonflat[0]) {
		if (nonflat[1]) {
			// One side is flat; 3 sub-triangles 
			Vector2f a, b, centerPoint, midpoint;
			a = vertices[sides[0]];
			b = vertices[(sides[0] + 1) % 3];
			centerPoint = vertices[(sides[0] + 2) % 3];
			midpoint = (a + b) / 2;

			Vector2f newVertices[3];
			newVertices[0] = a;
			newVertices[1] = midpoint;
			newVertices[2] = centerpoint;
			append(ret, tesselateTriangle(mode, center_test, newVertices, p, threshold));

			a = (b + centerpoint) / 2;
			newVertices[0] = b;
			newVertices[1] = midpoint;
			newVertices[2] = a;
			append(ret, tesselateTriangle(mode, center_test, newVertices, p, threshold));

			newVertices[0] = a;
			newVertices[1] = midpoint;
			newVertices[2] = centerPoint;
			append(ret, tesselateTriangle(mode, center_test, newVertices, p, threshold));
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
			append(ret, tesselateTriangle(mode, center_test, newVertices, p, threshold));

			newVertices[0] = b;
			newVertices[1] = midpoint;
			newVertices[2] = centerPoint;
			append(ret, tesselateTriangle(mode, center_test, newVertices, p, threshold));
		}
	} else {
		// All sides are flat; this triangle is a good polygon
		ret->push_back(new Triangle(p.evaluate(vertices[0]), p.evaluate(vertices[1]), p.evaluate(vertices[2])));
	}
	return ret;
}