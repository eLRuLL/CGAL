# This is the CMake script for compiling the CGAL Polyhedron demo.

project( Polyhedron )

cmake_minimum_required(VERSION 2.6.2)
if("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}" VERSION_GREATER 2.8.3)
  cmake_policy(VERSION 2.8.4)
else()
  cmake_policy(VERSION 2.6)
endif()

#option(POLYHEDRON_DEMO_ENABLE_FORWARD_DECL "In the Polyhedron demo, enable " OFF)
#mark_as_advanced(POLYHEDRON_DEMO_ENABLE_FORWARD_DECL)

# Let plugins be compiled in the same directory as the executable.
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

# Use packages improved since CGAL 3.4
foreach(DEP_PKG AABB_tree STL_Extension GraphicsView Surface_mesher Filtered_kernel Profiling_tools Mesh_3 Triangulation_3 )
  foreach(CGAL_SVN_TRUNK ../../../ ../../trunk ..)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${CGAL_SVN_TRUNK}/${DEP_PKG}/include")
      include_directories (BEFORE "${CMAKE_CURRENT_SOURCE_DIR}/${CGAL_SVN_TRUNK}/${DEP_PKG}/include")
      if(COMMAND break)
        break()
      endif()
    endif()
  endforeach()
endforeach()

# Include this package's headers first
include_directories( BEFORE ./ ./include ../../include ./CGAL_demo)

add_subdirectory( implicit_functions )

# Find CGAL and CGAL Qt4
find_package(CGAL COMPONENTS Qt4)
include( ${CGAL_USE_FILE} )

option( POLYHEDRON_QTSCRIPT_DEBUGGER 
  "Activate the use of Qt Script Debugger in Polyhedron_3 demo" OFF)

# Find Qt4 itself
set( QT_USE_QTXML    TRUE )
set( QT_USE_QTMAIN   TRUE )
set( QT_USE_QTSCRIPT  TRUE )
set( QT_USE_QTOPENGL  TRUE )
if( POLYHEDRON_QTSCRIPT_DEBUGGER)
  set( QT_USE_QTSCRIPTTOOLS  TRUE )
endif()
find_package(Qt4)

# Find OpenGL
find_package(OpenGL)

# Find QGLViewer
if(QT4_FOUND)
  include(${QT_USE_FILE})
  include_directories( ${QT_INCLUDE_DIR}/QtScriptTools )
  find_package(QGLViewer )
endif(QT4_FOUND)



# Eigen is now used by default
find_package(Eigen3 3.1.0) #(requires 3.1.0 or greater)
if (NOT EIGEN3_FOUND)
  # Find LAPACK (optional), for curvatures estimation
  find_package(LAPACK)
  if(LAPACK_FOUND)
    include( ${LAPACK_USE_FILE} )
  endif(LAPACK_FOUND)

  # Find TAUCS (optionnal), for parametrization
  find_package(TAUCS)
  if(TAUCS_FOUND)
    include( ${TAUCS_USE_FILE} )
  endif(TAUCS_FOUND)
else()
  include( ${EIGEN3_USE_FILE} )
endif()


if(CGAL_Qt4_FOUND AND QT4_FOUND AND OPENGL_FOUND AND QGLVIEWER_FOUND)

  set(Boost_USE_MULTITHREADED      ON)
  find_package(Boost COMPONENTS thread system)

  include_directories ( ${QGLVIEWER_INCLUDE_DIR} )

  # Parameterization needs Eigen3 or TAUCS
  if(NOT EIGEN3_FOUND AND NOT TAUCS_FOUND)
    message(STATUS "NOTICE: Eigen 3.1 (or greater) and TAUCS is not found. parametrization will not be available.")
  endif(NOT EIGEN3_FOUND AND NOT TAUCS_FOUND)

  # Curvature estimation needs Eigen3 or LAPACK
  if(NOT EIGEN3_FOUND AND NOT LAPACK_FOUND)
    message(STATUS "NOTICE: Eigen 3.1 (or greater) and LAPACK is not found. curvatures estimation will not be available.")
  endif(NOT EIGEN3_FOUND AND NOT LAPACK_FOUND)

  qt4_wrap_ui( MainWindowUI_files MainWindow.ui )
  qt4_wrap_ui( FileLoaderDialogUI_files FileLoaderDialog.ui )
  qt4_wrap_ui( Show_point_dialogUI_FILES Show_point_dialog.ui )
  qt4_wrap_ui( remeshingUI_FILES  Remeshing_dialog.ui)
  qt4_wrap_ui( meshingUI_FILES  Meshing_dialog.ui Meshing_pause_widget.ui )
  qt4_wrap_ui( cameraUI_FILES   Camera_positions_list.ui )
  qt4_wrap_ui( PreferencesUI_FILES   Preferences.ui )

  qt4_wrap_ui( funcUI_FILES Function_dialog.ui )

  qt4_generate_moc( "MainWindow.h" "${CMAKE_CURRENT_BINARY_DIR}/MainWindow_moc.cpp" )
  qt4_generate_moc( "File_loader_dialog.h" "${CMAKE_CURRENT_BINARY_DIR}/File_loader_dialog_moc.cpp" )
  add_file_dependencies( MainWindow_moc.cpp "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow.h" )
  add_file_dependencies( File_loader_dialog_moc_moc.cpp "${CMAKE_CURRENT_SOURCE_DIR}/File_loader_dialog.h" )

  qt4_generate_moc( "Viewer.h" "${CMAKE_CURRENT_BINARY_DIR}/Viewer_moc.cpp" )
  add_file_dependencies( Viewer_moc.cpp "${CMAKE_CURRENT_SOURCE_DIR}/Viewer.h" )

  include( ${CMAKE_CURRENT_SOURCE_DIR}/polyhedron_demo_macros.cmake )

  qt4_add_resources ( RESOURCE_FILES Polyhedron_3.qrc )

  qt4_generate_moc( "CGAL_demo/Viewer_interface.h" "${CMAKE_CURRENT_BINARY_DIR}/Viewer_interface_moc.cpp" )

  qt4_automoc(Scene.cpp
              Scene_item.cpp
              Scene_plane_item.cpp
              Scene_combinatorial_map_item.cpp
              Scene_polygon_soup_item.cpp
              Scene_polyhedron_item.cpp
              Scene_polyhedron_transform_item.cpp
              Scene_polylines_item.cpp
              Scene_edit_polyhedron_item.cpp
              Scene_textured_polyhedron_item.cpp
              Scene_c2t3_item.cpp
              Scene_nef_polyhedron_item.cpp
              Show_point_dialog.cpp
              Mesher_base.cpp
              Camera_positions_list.cpp
              Scene_points_with_normal_item.cpp
              )
  qt4_automoc( Scene_implicit_function_item.cpp )


  # AUXILIARY LIBRARIES

  # put plugins (which are shared libraries) at the same location as
  # executable files
  set(CGAL_POLYHEDRON_DEMO_PLUGINS_DIR "${RUNTIME_OUTPUT_PATH}")
  set(LIBRARY_OUTPUT_PATH "${CGAL_POLYHEDRON_DEMO_PLUGINS_DIR}")

  add_library(demo_framework SHARED
    Scene.cpp
    Viewer.cpp Viewer_moc.cpp Viewer_interface_moc.cpp
    Scene_item.cpp Scene_item.moc
    Scene_item_with_display_list.cpp
    Polyhedron_demo_plugin_helper.cpp)

  target_link_libraries(demo_framework
    ${QGLVIEWER_LIBRARIES}
    ${QT_LIBRARIES}
    ${OPENGL_gl_LIBRARY}
    ${OPENGL_glu_LIBRARY}
    )

  add_library(scene_basic_objects SHARED
    Scene_plane_item.cpp Scene_plane_item.moc)
  target_link_libraries(scene_basic_objects 
    demo_framework
    ${CGAL_LIBRARIES}
    ${QGLVIEWER_LIBRARIES}
    ${QT_LIBRARIES}
    ${OPENGL_gl_LIBRARY}
    ${OPENGL_glu_LIBRARY}
    )

  add_library(point_dialog SHARED Show_point_dialog.cpp Show_point_dialog.ui ${Show_point_dialogUI_FILES})
  target_link_libraries(point_dialog ${QT_LIBRARIES})

  macro(add_item item_name)
    add_library(${item_name} SHARED ${ARGN})
    target_link_libraries(${item_name} demo_framework ${QT_LIBRARIES} ${CGAL_LIBRARIES} ${Boost_LIBRARIES})
  endmacro(add_item)

  add_item(scene_c2t3_item Scene_c2t3_item.cpp Scene_c2t3_item.moc)
  add_item(scene_polyhedron_item Scene_polyhedron_item.cpp Scene_polyhedron_item.moc)
  add_item(scene_polyhedron_transform_item Scene_polyhedron_transform_item.cpp Scene_polyhedron_transform_item.moc)
  # special
  target_link_libraries(scene_polyhedron_transform_item scene_polyhedron_item)

  add_item(scene_combinatorial_map_item Scene_combinatorial_map_item.cpp Scene_combinatorial_map_item.moc)
  # special
  target_link_libraries(scene_combinatorial_map_item scene_polyhedron_item)

  add_item(scene_polylines_item Scene_polylines_item.cpp Scene_polylines_item.moc)

  add_item(scene_edit_polyhedron_item Scene_edit_polyhedron_item.cpp Scene_edit_polyhedron_item.moc)
  # special
  target_link_libraries(scene_edit_polyhedron_item scene_polyhedron_item)

  if(EIGEN3_FOUND OR TAUCS_FOUND)
    add_item(scene_textured_polyhedron_item Scene_textured_polyhedron_item.cpp texture.cpp Scene_textured_polyhedron_item.moc)
  endif(EIGEN3_FOUND OR TAUCS_FOUND)

  add_item(scene_implicit_function_item Scene_implicit_function_item.cpp Scene_implicit_function_item.moc Color_ramp.cpp)

  add_item(scene_polygon_soup_item Scene_polygon_soup_item.cpp Scene_polygon_soup_item.moc)
  target_link_libraries(scene_polygon_soup_item scene_polyhedron_item)
  add_item(scene_nef_polyhedron_item Scene_nef_polyhedron_item.cpp Scene_nef_polyhedron_item.moc
    Scene_nef_rendering.cpp)
  target_link_libraries(scene_nef_polyhedron_item scene_polyhedron_item)
  add_item(scene_points_with_normal_item Scene_points_with_normal_item.cpp Scene_points_with_normal_item.moc)

  foreach( lib 
      demo_framework
      scene_basic_objects
      scene_polyhedron_item
      scene_polygon_soup_item
      scene_nef_polyhedron_item )
    add_to_cached_list( CGAL_EXECUTABLE_TARGETS ${lib} )
  endforeach()

  add_definitions(-DQT_STATICPLUGIN)
#  if(POLYHEDRON_DEMO_ENABLE_FORWARD_DECL)
    set(CGAL_POLYHEDRON_DEMO_DEFINITIONS "-DUSE_FORWARD_DECL")
    add_definitions(${CGAL_POLYHEDRON_DEMO_DEFINITIONS})
    add_executable  ( Polyhedron_3 MainWindow.cpp
      Polyhedron_3.cpp
#      Viewer.cpp
#      Scene.cpp
#      MainWindow_curvature_estimation.cpp
      MainWindow_moc.cpp
      File_loader_dialog_moc.cpp
#      Viewer_moc.cpp
       ${FileLoaderDialogUI_files} ${MainWindowUI_files} ${PreferencesUI_FILES} ${RESOURCE_FILES} )
    add_to_cached_list( CGAL_EXECUTABLE_TARGETS Polyhedron_3 )
    if(EIGEN3_FOUND OR TAUCS_FOUND)
#      add_executable( Polyhedron_3 Scene_tex_rendering.cpp Scene_tex_polyhedron_operations.cpp )
    endif()
#  else(POLYHEDRON_DEMO_ENABLE_FORWARD_DECL)
#    add_file_dependencies( Polyhedron_3.cpp "${CMAKE_CURRENT_BINARY_DIR}/MainWindow_moc.cpp"
#                                            "${CMAKE_CURRENT_BINARY_DIR}/Scene_moc.cpp"
#                                            "${CMAKE_CURRENT_BINARY_DIR}/File_loader_dialog_moc.cpp"
#					    "${CMAKE_CURRENT_BINARY_DIR}/Viewer_moc.cpp" )
#    add_executable  ( Polyhedron_3 Polyhedron_3.cpp ${UI_FILES} ${RESOURCE_FILES} )
#  endif(POLYHEDRON_DEMO_ENABLE_FORWARD_DECL)


  # Link with Qt libraries
  if( POLYHEDRON_QTSCRIPT_DEBUGGER )
    add_definitions( -DQT_SCRIPTTOOLS_LIB )
    target_link_libraries( Polyhedron_3 QtScriptTools )
  endif()
  target_link_libraries( Polyhedron_3 ${QT_LIBRARIES} )
  target_link_libraries( Polyhedron_3 demo_framework )
  target_link_libraries( Polyhedron_3 point_dialog )

  # Link with CGAL
  target_link_libraries( Polyhedron_3 ${CGAL_LIBRARIES} ${CGAL_3RD_PARTY_LIBRARIES} )

  # Link with libQGLViewer, OpenGL
  target_link_libraries( Polyhedron_3 ${QGLVIEWER_LIBRARIES} ${OPENGL_gl_LIBRARY} ${OPENGL_glu_LIBRARY} )

  add_to_cached_list( CGAL_EXECUTABLE_TARGETS Polyhedron_3 )


  ###########
  # PLUGINS #
  ###########
  remove_definitions(-DQT_STATICPLUGIN)

  polyhedron_demo_plugin(remeshing_plugin Polyhedron_demo_remeshing_plugin Polyhedron_demo_remeshing_plugin_cgal_code.cpp Mesher_base.cpp ${remeshingUI_FILES})
  target_link_libraries(remeshing_plugin scene_polyhedron_item scene_polygon_soup_item scene_c2t3_item)

  polyhedron_demo_plugin(camera_positions_plugin 
    Polyhedron_demo_camera_positions_plugin
    Camera_positions_list 
    Camera_positions_list.moc
    ${cameraUI_FILES})

  target_link_libraries( camera_positions_plugin ${QGLVIEWER_LIBRARIES} demo_framework)

  if ( Boost_VERSION GREATER 103400 )
    qt4_generate_moc( "Polyhedron_demo_mesh_3_plugin_cgal_code.cpp" "${CMAKE_CURRENT_BINARY_DIR}/Scene_c3t3_item.moc" )

    polyhedron_demo_plugin(mesh_3_plugin Polyhedron_demo_mesh_3_plugin 
      Polyhedron_demo_mesh_3_plugin_cgal_code.cpp Scene_c3t3_item.moc)
    target_link_libraries(mesh_3_plugin scene_polyhedron_item ${QGLVIEWER_LIBRARIES} ${OPENGL_gl_LIBRARY} ${OPENGL_glu_LIBRARY})
  else( Boost_VERSION GREATER 103400 )
    message(STATUS "warning: the plugin mesh_3_plugin requires Boost>=1.34.1 and will not be compiled.")
  endif( Boost_VERSION GREATER 103400 )

  polyhedron_demo_plugin(inside_out_plugin Polyhedron_demo_inside_out_plugin)
  target_link_libraries(inside_out_plugin scene_polyhedron_item scene_polygon_soup_item)

  polyhedron_demo_plugin(off_plugin Polyhedron_demo_off_plugin)
  target_link_libraries(off_plugin scene_polyhedron_item scene_polygon_soup_item)

  polyhedron_demo_plugin(nef_io_plugin Polyhedron_demo_io_nef_plugin)
  target_link_libraries(nef_io_plugin  scene_nef_polyhedron_item)

  polyhedron_demo_plugin(polylines_io_plugin Polyhedron_demo_polylines_io_plugin)
  target_link_libraries(polylines_io_plugin scene_polylines_item)

  polyhedron_demo_plugin(io_implicit_function_plugin Io_implicit_function_plugin ${funcUI_FILES})
  target_link_libraries(io_implicit_function_plugin scene_implicit_function_item)

  polyhedron_demo_plugin(gocad_plugin Polyhedron_demo_gocad_plugin)
  target_link_libraries(gocad_plugin scene_polyhedron_item)

  polyhedron_demo_plugin(stl_plugin Polyhedron_demo_stl_plugin)
  target_link_libraries(stl_plugin scene_polyhedron_item)

  polyhedron_demo_plugin(xyz_plugin Polyhedron_demo_xyz_plugin)
  target_link_libraries(xyz_plugin scene_points_with_normal_item)

  polyhedron_demo_plugin(orient_soup_plugin Polyhedron_demo_orient_soup_plugin)
  target_link_libraries(orient_soup_plugin scene_polygon_soup_item scene_polyhedron_item)

  polyhedron_demo_plugin(triangulate_facets_plugin Polyhedron_demo_triangulate_facets_plugin)
  target_link_libraries(triangulate_facets_plugin scene_polyhedron_item)

  polyhedron_demo_plugin(off_to_nef_plugin Polyhedron_demo_off_to_nef_plugin)
  target_link_libraries(off_to_nef_plugin scene_nef_polyhedron_item)

  polyhedron_demo_plugin(off_to_xyz_plugin Polyhedron_demo_off_to_xyz_plugin)
  target_link_libraries(off_to_xyz_plugin scene_points_with_normal_item)

  polyhedron_demo_plugin(convex_hull_plugin Polyhedron_demo_convex_hull_plugin)
  target_link_libraries(convex_hull_plugin scene_polyhedron_item scene_points_with_normal_item scene_polylines_item)

  polyhedron_demo_plugin(kernel_plugin Polyhedron_demo_kernel_plugin)
  target_link_libraries(kernel_plugin scene_polyhedron_item)

  polyhedron_demo_plugin(pca_plugin Polyhedron_demo_pca_plugin)
  target_link_libraries(pca_plugin scene_polyhedron_item scene_basic_objects)

  if(EIGEN3_FOUND OR TAUCS_FOUND)
    polyhedron_demo_plugin(parameterization_plugin Polyhedron_demo_parameterization_plugin)
    target_link_libraries(parameterization_plugin scene_polyhedron_item scene_textured_polyhedron_item )
  else(EIGEN3_FOUND OR TAUCS_FOUND)
    message(STATUS "NOTICE: Neither TAUCS nor Eigen 3.1 (or greater) libraries have been found. Parameterization will not be available.")
  endif(EIGEN3_FOUND OR TAUCS_FOUND)
  
  if(TAUCS_FOUND OR EIGEN3_FOUND)
    qt4_wrap_ui( poissonUI_FILES Polyhedron_demo_poisson_plugin.ui)
    polyhedron_demo_plugin(poisson_plugin Polyhedron_demo_poisson_plugin Polyhedron_demo_poisson_plugin_impl.cpp ${poissonUI_FILES})
    target_link_libraries(poisson_plugin scene_polyhedron_item scene_points_with_normal_item)
  else(TAUCS_FOUND OR EIGEN3_FOUND)
    message(STATUS "NOTICE: Neither TAUCS nor Eigen 3.1 (or greater) libraries have been found. Poisson reconstruction will not be available.")
  endif(TAUCS_FOUND OR EIGEN3_FOUND)
  
  
  # Link with BLAS and LAPACK only (optional), for Jet Fitting
  if (NOT EIGEN3_FOUND AND LAPACK_FOUND)
    include( ${LAPACK_USE_FILE} )
  endif()
  
  if(EIGEN3_FOUND OR LAPACK_FOUND)
    qt4_wrap_ui( normal_estimationUI_FILES Polyhedron_demo_normal_estimation_plugin.ui)
    polyhedron_demo_plugin(normal_estimation_plugin Polyhedron_demo_normal_estimation_plugin ${normal_estimationUI_FILES})
    target_link_libraries(normal_estimation_plugin scene_points_with_normal_item)
    
    polyhedron_demo_plugin(point_set_smoothing_plugin Polyhedron_demo_point_set_smoothing_plugin)
    target_link_libraries(point_set_smoothing_plugin scene_points_with_normal_item)

    polyhedron_demo_plugin(point_set_average_spacing_plugin Polyhedron_demo_point_set_average_spacing_plugin)
    target_link_libraries(point_set_average_spacing_plugin scene_points_with_normal_item)
    
  else(EIGEN3_FOUND OR LAPACK_FOUND)
    message(STATUS "NOTICE: Nor Eigen 3.1 (or greater) nor LAPACK library were found. Normal estimation and smoothing will not be available.")
  endif(EIGEN3_FOUND OR LAPACK_FOUND)

  polyhedron_demo_plugin(self_intersection_plugin Polyhedron_demo_self_intersection_plugin)
  target_link_libraries(self_intersection_plugin scene_polyhedron_item)

  polyhedron_demo_plugin(subdivision_methods_plugin Polyhedron_demo_subdivision_methods_plugin)
  target_link_libraries(subdivision_methods_plugin scene_polyhedron_item)

  polyhedron_demo_plugin(mesh_simplification_plugin Polyhedron_demo_mesh_simplification_plugin)
  target_link_libraries(mesh_simplification_plugin scene_polyhedron_item)

  polyhedron_demo_plugin(nef_plugin Polyhedron_demo_nef_plugin)
  target_link_libraries(nef_plugin scene_nef_polyhedron_item)

  polyhedron_demo_plugin(intersection_plugin Polyhedron_demo_intersection_plugin)
  target_link_libraries(intersection_plugin  scene_polyhedron_item scene_polylines_item)

  polyhedron_demo_plugin(transform_polyhedron_plugin Polyhedron_demo_transform_polyhedron_plugin)
  target_link_libraries(transform_polyhedron_plugin  scene_polyhedron_item scene_polyhedron_transform_item)

  polyhedron_demo_plugin(corefinement_plugin Polyhedron_demo_corefinement_plugin)
  target_link_libraries(corefinement_plugin  scene_polyhedron_item scene_combinatorial_map_item scene_polylines_item)

  polyhedron_demo_plugin(trivial_plugin Polyhedron_demo_trivial_plugin)

  polyhedron_demo_plugin(edit_polyhedron_plugin Polyhedron_demo_edit_polyhedron_plugin)
  target_link_libraries(edit_polyhedron_plugin scene_polyhedron_item scene_edit_polyhedron_item)

  polyhedron_demo_plugin(cut_plugin Polyhedron_demo_cut_plugin)
  target_link_libraries(cut_plugin scene_polyhedron_item scene_basic_objects)
  
  qt4_wrap_ui(point_set_simplificationUI_FILES Polyhedron_demo_point_set_simplification_plugin.ui)
  polyhedron_demo_plugin(point_set_simplification_plugin Polyhedron_demo_point_set_simplification_plugin ${point_set_simplificationUI_FILES})
  target_link_libraries(point_set_simplification_plugin scene_points_with_normal_item)

  qt4_wrap_ui( ps_outliers_removal_UI_FILES Polyhedron_demo_point_set_outliers_removal_plugin.ui)
  polyhedron_demo_plugin(point_set_outliers_removal_plugin Polyhedron_demo_point_set_outliers_removal_plugin ${ps_outliers_removal_UI_FILES}) 
  target_link_libraries(point_set_outliers_removal_plugin scene_points_with_normal_item)

#
# Exporting
#
  if(TARGET CGAL_Qt4)
    export(TARGETS CGAL_Qt4 FILE polyhedron_demo_targets.cmake NAMESPACE Polyhedron_)
  else()
    export(TARGETS FILE polyhedron_demo_targets.cmake)
  endif()
  export(
    TARGETS
    demo_framework
    scene_polyhedron_item
    scene_points_with_normal_item
    scene_implicit_function_item
    scene_polylines_item
    NAMESPACE Polyhedron_
    APPEND FILE polyhedron_demo_targets.cmake)

  configure_file(CGAL_polyhedron_demoConfig.cmake.in  CGAL_polyhedron_demoConfig.cmake)

else (CGAL_Qt4_FOUND AND QT4_FOUND AND OPENGL_FOUND AND QGLVIEWER_FOUND)

  set(POLYHEDRON_MISSING_DEPS "")

  if(NOT CGAL_Qt4_FOUND)
    set(POLYHEDRON_MISSING_DEPS "the CGAL Qt4 library, ${POLYHEDRON_MISSING_DEPS}")
  endif()

  if(NOT QT4_FOUND)
    set(POLYHEDRON_MISSING_DEPS "Qt4, ${POLYHEDRON_MISSING_DEPS}")
  endif()

  if(NOT OPENGL_FOUND)
    set(POLYHEDRON_MISSING_DEPS "OpenGL, ${POLYHEDRON_MISSING_DEPS}")
  endif()

  if(NOT QGLVIEWER_FOUND)
    set(POLYHEDRON_MISSING_DEPS "QGLViewer, ${POLYHEDRON_MISSING_DEPS}")
  endif()

  message(STATUS "NOTICE: This demo requires ${POLYHEDRON_MISSING_DEPS}and will not be compiled.")

endif (CGAL_Qt4_FOUND AND QT4_FOUND AND OPENGL_FOUND AND QGLVIEWER_FOUND)
