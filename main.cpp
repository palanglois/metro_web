// STL
#include <vector>
#include <set>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>


// External
#include "Eigen/Core"
#include "Eigen/Dense"
#include "tinyply/tinyply.h"
#include "nanoflann/nanoflann.hpp"

//Emscripten
#ifdef EMCC
#include <emscripten/emscripten.h>
#endif

using namespace std;


struct float3 { float x, y, z; };
struct double3 { double x, y, z; };
struct uint3 { uint32_t x, y, z; };

typedef Eigen::Vector3d Point;
typedef Eigen::Matrix<double, Eigen::Dynamic, 3> PointCloud;
typedef nanoflann::KDTreeEigenMatrixAdaptor<PointCloud> EigenKdTree;

class Triangle
{
public:
    Triangle();
    Triangle(Point _a, Point _b, Point _c);
    double getArea() const;
    const Point& getA() const;
    const Point& getB() const;
    const Point& getC() const;
private:
    const Point a;
    const Point b;
    const Point c;
};

using namespace tinyply;
using namespace Eigen;

Triangle::Triangle() : a(Point(0., 0., 0.)), b(Point(0., 0., 0.)), c(Point(0., 0., 0.))
{

}

Triangle::Triangle(Point _a, Point _b, Point _c) : a(move(_a)), b(move(_b)), c(move(_c))
{

}

double Triangle::getArea() const
{
    return 0.5*(b-a).cross(c-a).norm();
}

const Point& Triangle::getA() const
{
    return a;
}

const Point& Triangle::getB() const
{
    return b;
}

const Point& Triangle::getC() const
{
    return c;
}

vector<Triangle> loadTrianglesFromPly(istream& ss)
{
    try
    {
        PlyFile file;
        file.parse_header(ss);

        shared_ptr<PlyData> vertices, normals, faces, texcoords;

        try { vertices = file.request_properties_from_element("vertex", { "x", "y", "z" }); }
        catch (const exception & e) { cerr << "tinyply exception: " << e.what() << endl; }

        try 
        { 
          faces = file.request_properties_from_element("face", { "vertex_index" }); 
        }
        catch (const exception & e) 
        {
          try 
          { 
            faces = file.request_properties_from_element("face", { "vertex_indices" }); 
          } 
          catch (const exception & e)
          {
            cout << "tinyply exception: " << e.what() << endl; 
          }
        }

        file.read(ss);

        const size_t numVerticesBytes = vertices->buffer.size_bytes();
        vector<float3> verts(vertices->count);
        if(vertices->t == Type::FLOAT64)
        {
            vector<double3> vertsTemp(vertices->count);
            memcpy(vertsTemp.data(), vertices->buffer.get(), numVerticesBytes);
            for(int i=0; i<vertsTemp.size(); i++)
            {
                verts[i].x = static_cast<float>(vertsTemp[i].x);
                verts[i].y = static_cast<float>(vertsTemp[i].y);
                verts[i].z = static_cast<float>(vertsTemp[i].z);
            }
        }
        else
            memcpy(verts.data(), vertices->buffer.get(), numVerticesBytes);

        const size_t numFacesBytes = faces->buffer.size_bytes();
        vector<uint3> facs(faces->count);
        memcpy(facs.data(), faces->buffer.get(), numFacesBytes);

        vector<Triangle> outTriangles;
        for (auto &faceCoord : facs) {
            Point a(verts[faceCoord.x].x, verts[faceCoord.x].y, verts[faceCoord.x].z);
            Point b(verts[faceCoord.y].x, verts[faceCoord.y].y, verts[faceCoord.y].z);
            Point c(verts[faceCoord.z].x, verts[faceCoord.z].y, verts[faceCoord.z].z);
            outTriangles.emplace_back(a, b, c);
        }
        return outTriangles;
    }
    catch (const exception & e)
    {
        cerr << "Caught tinyply exception: " << e.what() << endl;
        return vector<Triangle>(0);
    }
}

PointCloud samplePointsOnMesh(const vector<Triangle>& mesh, int nbSamples)
{
    // Build the cumulated areas histogram
    vector<double> cumulatedAreas;
    double accum(0);
    for(const auto triangle: mesh)
    {
        accum += triangle.getArea();
        cumulatedAreas.push_back(accum);
    }
    // Normalize it
    for(double &cumulatedArea: cumulatedAreas)
        cumulatedArea /= accum;
    // Actual sampling
    PointCloud sampledPoints(nbSamples, 3);
#pragma omp parallel for
    for(int i=0; i < nbSamples; i++)
    {
        // Select a random triangle according to the areas distribution
        double r = ((double) rand() / (RAND_MAX));
        size_t found_index = 0;
        for(size_t j=0; j<cumulatedAreas.size() && r > cumulatedAreas[j]; j++)
            found_index = j+1;

        // Draw a random point in this triangle
        double r1 = ((double) rand() / (RAND_MAX));
        double r2 = ((double) rand() / (RAND_MAX));
        Point A = mesh[found_index].getA();
        Point B = mesh[found_index].getB();
        Point C = mesh[found_index].getC();
        Point P = (1 - sqrt(r1)) * A + (sqrt(r1) * (1 - r2)) * B + (sqrt(r1) * r2) * C;
#pragma omp critical
        sampledPoints.row(i) = P;
    }
    return sampledPoints;
}

multiset<double> findPcDistance(const PointCloud& refPointCloud, const PointCloud& queryPointCloud)
{
    // Build the kd-tree for the reference Point Cloud
    const int dim = 3;
    const int maxLeaf = 10;
    EigenKdTree refTree(dim, cref(refPointCloud), maxLeaf);

    // do a knn search
    const size_t num_results = 1;
    multiset<double> allDistances;
#pragma omp parallel for
    for(int i=0; i < queryPointCloud.rows(); i++)
    {
        vector<size_t> ret_indexes(num_results);
        vector<double> out_dists_sqr(num_results);
        nanoflann::KNNResultSet<double> resultSet(num_results);
        resultSet.init(&ret_indexes[0], &out_dists_sqr[0]);
        Point queryPoint = queryPointCloud.row(i);
        refTree.index->findNeighbors(resultSet, &queryPoint[0],
                                       nanoflann::SearchParams(10));
#pragma omp critical
        allDistances.insert(sqrt(out_dists_sqr[0]));
    }
    return allDistances;
}

#ifdef __cplusplus
extern "C" {
#endif

#ifdef EMCC
int EMSCRIPTEN_KEEPALIVE compute_metro(char* path1, int size1, char* path2, int size2, int nbSample)
#else
int compute_metro(char* path1, int size1, char* path2, int size2, int nbSample)
#endif
{
    string sFileOne(path1, size1);
    istringstream issOne(sFileOne);
    istream& fileOne(issOne);

    string sFileTwo(path2, size2);
    istringstream issTwo(sFileTwo);
    istream& fileTwo(issTwo);

    // Random seed
    srand(0);

    // Load the meshes
    vector<Triangle> meshOne = loadTrianglesFromPly(fileOne);
    vector<Triangle> meshTwo = loadTrianglesFromPly(fileTwo);

    // Sample the points
    PointCloud sampledOne = samplePointsOnMesh(meshOne, nbSample);
    PointCloud sampledTwo = samplePointsOnMesh(meshTwo, nbSample);

    // Retrieve the NN distances
    multiset<double> nnDistancesOne = findPcDistance(sampledOne, sampledTwo);
    multiset<double> nnDistancesTwo = findPcDistance(sampledTwo, sampledOne);

    // Output the results
    const double percentage = 0.95;
    cout << "# Number of point sampled on each mesh : " << nbSample << endl;
    cout << "# Distance d such as 95% NN distances are under d" << endl;
    cout << "# Mesh 1 reference; Mesh 2 query (precision if Mesh 1 is ground truth) : " << endl;
    auto itOne = nnDistancesOne.begin();
    advance(itOne, int(percentage*(nnDistancesOne.size()-1)));
    cout << *itOne << endl;
    cout << "# Mesh 2 reference; Mesh 1 query (completeness if Mesh 1 is ground truth) : " << endl;
    auto itTwo = nnDistancesTwo.begin();
    advance(itTwo, int(percentage*(nnDistancesTwo.size()-1)));
    cout << *itTwo << endl;
    cout << "# Average of all measurement (mean metro)" << endl;
    double average = 0.;
    for(const auto &measurement : nnDistancesOne)
        average += measurement;
    for(const auto &measurement : nnDistancesTwo)
        average += measurement;
    cout << average / double(nnDistancesOne.size() + nnDistancesTwo.size()) << endl;
    cout << "# Max measurement (metro distance)" << endl;
    cout << max(*nnDistancesOne.rbegin(), *nnDistancesTwo.rbegin()) << endl;

    return 0;
}

#ifdef __cplusplus
}
#endif

#ifndef EMCC
int main(int argc, char* argv[])
{
  if(argc != 4)
    {
        cerr << "Error : there should be exactly 3 arguments; example: ./metro_pp ref_mesh.ply query_mesh.ply 10000"
                << endl;
        return 1;
    }
    string fileOne = argv[1];
    string fileTwo = argv[2];
    if(fileOne.substr(fileOne.length()-3, 3) != "ply" && fileOne.substr(fileOne.length()-3, 3) != "PLY")
    {
        cerr << "Error the 1st file should end with ply or PLY !" << endl;
        return 1;
    }
    if(fileTwo.substr(fileTwo.length()-3, 3) != "ply" && fileTwo.substr(fileTwo.length()-3, 3) != "PLY")
    {
        cerr << "Error the 2nd file should end with ply or PLY !" << endl;
        return 1;
    }
    int nbSample;
    try
    {
        nbSample = stoi(argv[3]);
    }
    catch (invalid_argument) {
        cerr << "Could not convert number of samples to an integer !\n";
        return 1;
    }
    
    ifstream fileOneContent(fileOne);
    ifstream fileTwoContent(fileTwo);

    string strOne((istreambuf_iterator<char>(fileOneContent)),
                 istreambuf_iterator<char>());
    string strTwo((istreambuf_iterator<char>(fileTwoContent)),
                 istreambuf_iterator<char>());

    compute_metro((char*)strOne.c_str(), strOne.size(), (char*)strTwo.c_str(), strTwo.size(), nbSample);

}
#endif
