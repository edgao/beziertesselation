#include <float.h>
#include "Class.h"
#include <iostream>
#include <cstdio>

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
bool BezierPatch::isFlat(Vector2f u, Vector2f v, float threshold) {
	Vector3f a, mid, b;
	a = evaluate(u);
	b = evaluate(v);
	mid = evaluate((u + v) / 2);
	return ((a + b) / 2 - mid).norm() < threshold;
}

Triangle::Triangle() {}
Triangle::Triangle(Vector3f v[]) {
	for (int i = 0; i < 3; i++) {
		vertices[i] = v[i];
	}
}

BezierPatchTesselator::BezierPatchTesselator() {}
BezierPatchTesselator::BezierPatchTesselator(vector<BezierPatch*>* p) {
	patches = *p;
}
vector<Triangle*>* BezierPatchTesselator::tesselate(int mode, bool centerTest) {
	vector<Triangle*>* triangles = (vector<Triangle*>*) malloc(sizeof(vector<Triangle*>));
	for (int patchNum = 0; patchNum < patches.size(); patchNum++) {
		Vector2f vertices[3];
		vertices[0] = (Vector2f() << 0, 0).finished();
		vertices[1] = (Vector2f() << 0, 1).finished();
		vertices[2] = (Vector2f() << 1, 0).finished();
		vector<Triangle*>* temp = tesselateTriangle(mode, center_test, vertices);
		for (int i = 0; i < temp->length; i++) {
			triangles->push_back((*temp)[i]);
		}

		vertices[0] = (Vector2f() << 1, 1).finished();
		vertices[1] = (Vector2f() << 0, 1).finished();
		vertices[2] = (Vector2f() << 1, 0).finished();
		vector<Triangle*>* temp = tesselateTriangle(mode, center_test, vertices);
		for (int i = 0; i < temp->length; i++) {
			triangles->push_back((*temp)[i]);
		}
	}
	return triangles;
}

vector<Triangle*>* BezierPatchTesselator::tesselateTriangle(int mode, bool center_test, Vector2f vertices[]) {

	return NULL;
}