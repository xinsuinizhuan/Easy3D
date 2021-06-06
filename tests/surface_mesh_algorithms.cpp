/********************************************************************
 * Copyright (C) 2015 Liangliang Nan <liangliang.nan@gmail.com>
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++
 *      library for processing and rendering 3D data. 2018.
 * ------------------------------------------------------------------
 * Easy3D is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 3
 * as published by the Free Software Foundation.
 *
 * Easy3D is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 ********************************************************************/

#include <easy3d/core/point_cloud.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/poly_mesh.h>
#include <easy3d/algo/surface_mesh_components.h>
#include <easy3d/algo/surface_mesh_curvature.h>
#include <easy3d/algo/surface_mesh_enumerator.h>
#include <easy3d/algo/surface_mesh_fairing.h>
#include <easy3d/algo/surface_mesh_geodesic.h>
#include <easy3d/algo/surface_mesh_hole_filling.h>
#include <easy3d/algo/surface_mesh_parameterization.h>
#include <easy3d/algo/surface_mesh_polygonization.h>
#include <easy3d/algo/surface_mesh_remeshing.h>
#include <easy3d/algo/surface_mesh_sampler.h>
#include <easy3d/algo/surface_mesh_simplification.h>
#include <easy3d/algo/surface_mesh_smoothing.h>
#include <easy3d/algo/surface_mesh_stitching.h>
#include <easy3d/algo/surface_mesh_subdivision.h>
#include <easy3d/algo/surface_mesh_tetrahedralization.h>
#include <easy3d/algo/surface_mesh_topology.h>
#include <easy3d/algo/surface_mesh_triangulation.h>
#include <easy3d/algo/surface_mesh_features.h>
#include <easy3d/fileio/surface_mesh_io.h>
#include <easy3d/fileio/resources.h>

#if HAS_CGAL
#include <easy3d/algo_ext/surfacer.h>
#endif


using namespace easy3d;

bool test_algo_surface_mesh_components() {
    const std::string file = resource::directory() + "/data/house/house.obj";
    SurfaceMesh *mesh = SurfaceMeshIO::load(file);
    if (!mesh) {
        std::cerr << "Error: failed to load model. Please make sure the file exists and format is correct."
                  << std::endl;
        return false;
    }

    const std::vector<SurfaceMeshComponent> &components = SurfaceMeshComponent::extract(mesh);
    std::cout << "mesh has " << components.size() << " components." << std::endl;
    for (std::size_t i = 0; i < std::min(std::size_t(10), components.size()); ++i) {
        const auto &comp = components[i];
        std::cout << "\tcomponent " << i << ": \n"
                  << "\t\t#faces: " << comp.n_faces() << "\n"
                  << "\t\t#vertices: " << comp.n_vertices() << "\n"
                  << "\t\t#edges: " << comp.n_edges() << "\n"
                  << "\t\tarea: " << comp.area() << "\n"
                  << "\t\tborder_length: " << comp.border_length() << "\n";
    }
    return true;
}


bool test_algo_surface_mesh_curvature() {
    const std::string file = resource::directory() + "/data/mannequin.ply";
    SurfaceMesh *mesh = SurfaceMeshIO::load(file);
    if (!mesh) {
        std::cerr << "Error: failed to load model. Please make sure the file exists and format is correct."
                  << std::endl;
        return false;
    }

    SurfaceMeshCurvature analyzer(mesh);

    std::cout << "computing surface mesh principle curvatures...";
    analyzer.analyze_tensor(2, true);
    std::cout << " success" << std::endl;

    std::cout << "computing surface mesh mean curvatures...";
    analyzer.compute_mean_curvature();
    std::cout << " success" << std::endl;

    std::cout << "computing surface mesh Gauss curvatures...";
    analyzer.compute_gauss_curvature();
    std::cout << " success" << std::endl;

    std::cout << "computing surface mesh max absolute curvatures...";
    analyzer.compute_max_abs_curvature();
    std::cout << " success" << std::endl;

    return true;
}


bool test_algo_surface_mesh_enumerator() {
    const std::string file = resource::directory() + "/data/house/house.obj";
    SurfaceMesh *mesh = SurfaceMeshIO::load(file);
    if (!mesh) {
        std::cerr << "Error: failed to load model. Please make sure the file exists and format is correct."
                  << std::endl;
        return false;
    }

    std::cout << "enumerating connected components...";
    auto connected_components = mesh->face_property<int>("f:connected_component", -1);
    SurfaceMeshEnumerator::enumerate_connected_components(mesh, connected_components);
    std::cout << " success" << std::endl;

    std::cout << "enumerating planar components...";
    auto planar_segments = mesh->face_property<int>("f:planar_partition", -1);
    SurfaceMeshEnumerator::enumerate_planar_components(mesh, planar_segments, 1.0f);
    std::cout << " success" << std::endl;

    return true;
}


bool test_algo_surface_mesh_fairing() {
    const std::string file = resource::directory() + "/data/hemisphere.ply";
    SurfaceMesh *mesh = SurfaceMeshIO::load(file);
    if (!mesh) {
        std::cerr << "Error: failed to load model. Please make sure the file exists and format is correct."
                  << std::endl;
        return false;
    }

    std::cout << "fairing by minimizing area ...";
    {
        SurfaceMeshFairing fair(mesh);
        fair.minimize_area(); // Minimize Area
        std::cout << " success" << std::endl;
    }

    std::cout << "fairing by minimizing curvature ...";
    {
        SurfaceMeshFairing fair(mesh);
        fair.minimize_curvature();
        std::cout << " success" << std::endl;
    }

    std::cout << "fairing by minimizing curvature variation...";
    {
        SurfaceMeshFairing fair(mesh);
        fair.fair(3);
        std::cout << " success" << std::endl;
    }

    return true;
}


bool test_algo_surface_mesh_geodesic() {
    const std::string file = resource::directory() + "/data/mannequin.ply";
    SurfaceMesh *mesh = SurfaceMeshIO::load(file);
    if (!mesh) {
        std::cerr << "Error: failed to load model. Please make sure the file exists and format is correct."
                  << std::endl;
        return false;
    }

    std::cout << "computing geodesic distance from the first vertex...";
    std::vector<SurfaceMesh::Vertex> seeds(1, SurfaceMesh::Vertex(0));

    // compute geodesic distance
    SurfaceMeshGeodesic geodist(mesh);
    geodist.compute(seeds);
    std::cout << " success" << std::endl;

    return true;
}


bool test_algo_surface_mesh_fill_holes() {
    const std::string file = resource::directory() + "/data/bunny.ply";
    SurfaceMesh *mesh = SurfaceMeshIO::load(file);
    if (!mesh) {
        std::cerr << "Error: failed to load model. Please make sure the file exists and format is correct."
                  << std::endl;
        return false;
    }

    // first, find all the boundaries to be filled
    const int allowed_boundary_size = 500;

    std::vector<std::pair<SurfaceMesh::Halfedge, int> > holes;

    auto visited = mesh->add_halfedge_property<bool>("TestSurfaceMeshHoleFilling::h::visited", false);
    for (auto h : mesh->halfedges()) {
        if (!visited[h] && mesh->is_border(h)) {
            int size = 0;
            SurfaceMesh::Halfedge hh = h;
            do {
                visited[hh] = true;
                ++size;
                if (!mesh->is_manifold(mesh->target(hh))) {
                    size += 123456;
                    break;
                }
                hh = mesh->next(hh);
            } while (hh != h);

            if (size < allowed_boundary_size) {
                holes.push_back({h, size});
            }
        }
    }
    mesh->remove_halfedge_property(visited);

    std::cout << "filling holes... ";
    // close holes whose sizes are smaller than the min allowed boundary size
    int num_closed = 0;
    for (const auto &hole : holes) {
        SurfaceMeshHoleFilling hf(mesh);
        hf.fill_hole(hole.first);
        ++num_closed;
    }
    std::cout << num_closed << " (out of " << holes.size() << ") holes filled" << std::endl;

    return true;
}


bool test_algo_surface_mesh_parameterization() {
    const std::string file = resource::directory() + "/data/mannequin.ply";
    SurfaceMesh *mesh = SurfaceMeshIO::load(file);
    if (!mesh) {
        std::cerr << "Error: failed to load model. Please make sure the file exists and format is correct."
                  << std::endl;
        return false;
    }

    SurfaceMeshParameterization para(mesh);

    std::cout << "parameterization (Least Squares Conformal Map) ...";
    para.lscm();
    std::cout << " success" << std::endl;

    std::cout << "parameterization (Discrete Harmonic) ...";
    para.harmonic();
    std::cout << " success" << std::endl;

    return true;
}


bool test_algo_surface_mesh_polygonization() {
    const std::string file = resource::directory() + "/data/fandisk.off";
    SurfaceMesh *mesh = SurfaceMeshIO::load(file);
    if (!mesh) {
        std::cerr << "Error: failed to load model. Please make sure the file exists and format is correct."
                  << std::endl;
        return false;
    }

    std::cout << "polygonization...";

#if HAS_CGAL
    // stitch first: to encourage large polyons
    Surfacer::stitch_borders(mesh);
    Surfacer::merge_reversible_connected_components(mesh);
#endif

    // polygonization
    SurfaceMeshPolygonization polygonizer;
    polygonizer.apply(mesh);
    std::cout << " success" << std::endl;

#if HAS_CGAL
    // stitch again (the "merge-edge" edge operation in polygonization may result in some borders)
    Surfacer::stitch_borders(mesh);
    Surfacer::merge_reversible_connected_components(mesh);
#endif

    return true;
}


bool test_algo_surface_mesh_remeshing() {
    const std::string file = resource::directory() + "/data/bunny.ply";
    SurfaceMesh *mesh = SurfaceMeshIO::load(file);
    if (!mesh) {
        std::cerr << "Error: failed to load model. Please make sure the file exists and format is correct."
                  << std::endl;
        return false;
    }

    { // detect features
        SurfaceMeshFeatures sf(mesh);
        sf.clear();
        sf.detect_angle(60.0f);
        sf.detect_boundary();
    }

    std::cout << "uniform remeshing...";
    {
        float len(0.0f);
        for (auto eit : mesh->edges())
            len += distance(mesh->position(mesh->vertex(eit, 0)),
                            mesh->position(mesh->vertex(eit, 1)));
        len /= (float) mesh->n_edges();
        SurfaceMeshRemeshing(mesh).uniform_remeshing(len);
        std::cout << " success" << std::endl;
    }

    std::cout << "adaptive remeshing...";
    {
        auto bb = mesh->bounding_box().diagonal();
        SurfaceMeshRemeshing(mesh).adaptive_remeshing(
                0.001 * bb,  // min length
                0.100 * bb,  // max length
                0.001 * bb); // approx. error
        std::cout << " success" << std::endl;
    }

    return true;
}


bool test_algo_surface_mesh_sampler() {
    const std::string file = resource::directory() + "/data/bunny.ply";
    SurfaceMesh *mesh = SurfaceMeshIO::load(file);
    if (!mesh) {
        std::cerr << "Error: failed to load model. Please make sure the file exists and format is correct."
                  << std::endl;
        return false;
    }

    std::cout << "sampling surface mesh...";
    SurfaceMeshSampler sampler;
    PointCloud *cloud = sampler.apply(mesh, 100000);
    if (cloud) {
        std::cout << " success" << std::endl;
        delete cloud;
        return true;
    }
    return false;
}


bool test_algo_surface_mesh_simplification() {
    const std::string file = resource::directory() + "/data/bunny.ply";
    SurfaceMesh *mesh = SurfaceMeshIO::load(file);
    if (!mesh) {
        std::cerr << "Error: failed to load model. Please make sure the file exists and format is correct."
                  << std::endl;
        return false;
    }

    std::cout << "simplification of surface mesh...";
    const int normal_deviation = 180;
    const int aspect_ratio = 10;

    const int expected_vertex_number = mesh->n_vertices() * 0.5f;
    SurfaceMeshSimplification ss(mesh);
    ss.initialize(aspect_ratio, 0.0, 0.0, normal_deviation, 0.0);
    ss.simplify(expected_vertex_number);
    std::cout << " success" << std::endl;

    return true;
}


bool test_algo_surface_mesh_smoothing() {
    const std::string file = resource::directory() + "/data/bunny.ply";
    SurfaceMesh *mesh = SurfaceMeshIO::load(file);
    if (!mesh) {
        std::cerr << "Error: failed to load model. Please make sure the file exists and format is correct."
                  << std::endl;
        return false;
    }

    std::cout << "explicit smoothing...";
    {
        SurfaceMeshSmoothing smoother(mesh);
        smoother.explicit_smoothing(2, true);
        std::cout << " success" << std::endl;
    }

    std::cout << "implicit smoothing...";
    {
        const float timestep = 0.001f;

        // does the mesh have a boundary?
        bool has_boundary = false;
        for (auto v: mesh->vertices())
            if (mesh->is_border(v))
                has_boundary = true;

        // only re-scale if we don't have a (fixed) boundary
        const bool rescale = !has_boundary;

        SurfaceMeshSmoothing smoother(mesh);
        smoother.implicit_smoothing(timestep, true, rescale);
        std::cout << " success" << std::endl;
    }

    return true;
}


bool test_algo_surface_mesh_stitching() {
    const std::string file = resource::directory() + "/data/house/house.obj";
    SurfaceMesh *mesh = SurfaceMeshIO::load(file);
    if (!mesh) {
        std::cerr << "Error: failed to load model. Please make sure the file exists and format is correct."
                  << std::endl;
        return false;
    }

    std::cout << "stitching surface mesh...";

#if HAS_CGAL
    Surfacer::stitch_borders(mesh);
    Surfacer::merge_reversible_connected_components(mesh);
    std::cout << " success" << std::endl;
#else
    SurfaceMeshStitching stitch(mesh);
    stitch.apply();
    std::cout << " success" << std::endl;
#endif

    return true;
}


bool test_algo_surface_mesh_subdivision() {
    const std::string file = resource::directory() + "/data/sphere.obj";
    SurfaceMesh *mesh = SurfaceMeshIO::load(file);
    if (!mesh) {
        std::cerr << "Error: failed to load model. Please make sure the file exists and format is correct."
                  << std::endl;
        return false;
    }

    std::cout << "Loop subdivision...";
    if (!SurfaceMeshSubdivision::loop(mesh))
        return false;
    std::cout << " success" << std::endl;

    std::cout << "Sqrt3 subdivision...";
    if (!SurfaceMeshSubdivision::catmull_clark(mesh))
        return false;
    std::cout << " success" << std::endl;

    std::cout << "CatmullClark subdivision...";
    if (!SurfaceMeshSubdivision::catmull_clark(mesh))
        return false;
    std::cout << " success" << std::endl;

    return true;
}


bool test_algo_surface_mesh_tetrahedralization() {
    const std::string file = resource::directory() + "/data/sphere.obj";
    SurfaceMesh *mesh = SurfaceMeshIO::load(file);
    if (!mesh) {
        std::cerr << "Error: failed to load model. Please make sure the file exists and format is correct."
                  << std::endl;
        return false;
    }

    std::cout << "tetrehedralization...";
    SurfaceMeshTetrehedralization tetra;
    PolyMesh* result = tetra.apply(mesh);
    if (result) {
        std::cout << " success" << std::endl;
        delete result;
        return true;
    }

    return false;
}


bool test_algo_surface_mesh_topology() {
    const std::string file = resource::directory() + "/data/house/house.obj";
    SurfaceMesh *mesh = SurfaceMeshIO::load(file);
    if (!mesh) {
        std::cerr << "Error: failed to load model. Please make sure the file exists and format is correct."
                  << std::endl;
        return false;
    }

    std::cout << "computing surface mesh topology..." << std::endl;

    const auto &components = SurfaceMeshComponent::extract(mesh);
    std::cout << "model has " << components.size() << " connected components" << std::endl;


    const std::size_t num = 10;
    if (components.size() > num)
        std::cout << "    topology of the first " << num << " components:" << std::endl;

    for (std::size_t i = 0; i < std::min(components.size(), num); ++i) {
        const SurfaceMeshComponent &comp = components[i];
        SurfaceMeshTopology topo(&comp);
        std::string type = "unknown";
        if (topo.is_sphere())
            type = "sphere";
        else if (topo.is_disc())
            type = "disc";
        else if (topo.is_cylinder())
            type = "cylinder";
        else if (topo.is_torus())
            type = "torus";
        else if (topo.is_closed())
            type = "unknown closed";

        std::cout << "        " << i << ": " << type
               << ", F = " << comp.n_faces() << ", V = " << comp.n_vertices() << ", E = " << comp.n_edges()
               << ", B = " << topo.number_of_borders();
        if (topo.number_of_borders() == 1)
            std::cout << ", border size = " << topo.largest_border_size();
        else if (topo.number_of_borders() > 1)
            std::cout << ", largest border size = " << topo.largest_border_size();
        std::cout << std::endl;
    }

    return true;
}


bool test_algo_surface_mesh_triangulation() {
    const std::string file = resource::directory() + "/data/girl_face.ply";
    SurfaceMesh *mesh = SurfaceMeshIO::load(file);
    if (!mesh) {
        std::cerr << "Error: failed to load model. Please make sure the file exists and format is correct."
                  << std::endl;
        return false;
    }

    std::cout << "triangulating surface mesh...";

    SurfaceMeshTriangulation triangulator(mesh);
    triangulator.triangulate(SurfaceMeshTriangulation::MIN_AREA);
    std::cout << " success" << std::endl;

    return true;
}


int test_surface_mesh_algorithms() {
    if (!test_algo_surface_mesh_components())
        return EXIT_FAILURE;

    if (!test_algo_surface_mesh_curvature())
        return EXIT_FAILURE;

    if (!test_algo_surface_mesh_enumerator())
        return EXIT_FAILURE;

    if (!test_algo_surface_mesh_fairing())
        return EXIT_FAILURE;

    if (!test_algo_surface_mesh_geodesic())
        return EXIT_FAILURE;

    if (!test_algo_surface_mesh_fill_holes())
        return EXIT_FAILURE;

    if (!test_algo_surface_mesh_parameterization())
        return EXIT_FAILURE;

    if (!test_algo_surface_mesh_polygonization())
        return EXIT_FAILURE;

    if (!test_algo_surface_mesh_remeshing())
        return EXIT_FAILURE;

    if (!test_algo_surface_mesh_sampler())
        return EXIT_FAILURE;

    if (!test_algo_surface_mesh_simplification())
        return EXIT_FAILURE;

    if (!test_algo_surface_mesh_smoothing())
        return EXIT_FAILURE;

    if (!test_algo_surface_mesh_stitching())
        return EXIT_FAILURE;

    if (!test_algo_surface_mesh_subdivision())
        return EXIT_FAILURE;

    if (!test_algo_surface_mesh_tetrahedralization())
        return EXIT_FAILURE;

    if (!test_algo_surface_mesh_topology())
        return EXIT_FAILURE;

    if (!test_algo_surface_mesh_triangulation())
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}