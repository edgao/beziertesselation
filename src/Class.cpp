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

BezierPatch::BezierPatch() {}
BezierPatch::BezierPatch(BezierCurve c[]) {
	for (int i = 0; i < 4; i++) {
		curves[i] = c[i];
	}
}