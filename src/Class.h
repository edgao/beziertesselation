#include <eigen3/Dense>
#include <vector>

using namespace Eigen;
using namespace std;

class BezierCurve {
public:
	Vector3f points[4];
	BezierCurve();
	BezierCurve(Vector3f[]);
	Vector3f evaluate(float);
};

class BezierPatch {
public:
	BezierCurve curves[4];
	BezierPatch();
	BezierPatch(BezierCurve[]);
	Vector3f evaluate(Vector2f);
    Vector3f findNormal(Vector2f);
};

class Triangle {
public:
	Vector3f vertices[3];
	Vector3f normals[3];
	Triangle();
	Triangle(Vector3f, Vector3f, Vector3f, Vector3f, Vector3f, Vector3f);
	Triangle(Vector3f[], Vector3f[]);
	Vector3f getFlatNormal();
};

class BezierPatchTesselator {
public:
	static const int ADAPTIVE_MODE = 0, UNIFORM_MODE = 1;
	vector<BezierPatch*> patches;
	BezierPatchTesselator();
	BezierPatchTesselator(vector<BezierPatch*>*);
	vector<Triangle*>* tesselate(int mode, bool center_test, float threshold);
	vector<Triangle*>* tesselateTriangle(int mode, bool center_test, Vector2f vertices[], BezierPatch p, float threshold);
	bool isFlat(BezierPatch, Vector3f, Vector3f, float);
};
