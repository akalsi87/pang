# executable : pang

# -- Headers

set(pang_int_hdr
    include/pang/types.hpp;
    include/pang/game.hpp)

# -- Sources
set(pang_src
    src/pang/main.cpp;
    src/pang/game.cpp)

set(sfml_dir ${CMAKE_CURRENT_SOURCE_DIR}/external/SFML-2.3.2)

# -- Targets
# --- executable
add_exe(pang ${pang_src} ${pang_int_hdr})
add_inc_dir(pang ${sfml_dir}/include)
link_libs(pang )
add_link_flag(pang -Wl,-L${sfml_dir}/lib)
add_link_lib(pang sfml-window)
add_link_lib(pang sfml-system)
add_link_lib(pang sfml-graphics)

add_dependencies(check pang)

# --- move our deps into CURRENT_BINARY_DIR so we find it at runtime
file(COPY ${sfml_dir}/lib/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY resources DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# -- Install!
# --- install our deps
install(DIRECTORY ${sfml_dir}/lib/ DESTINATION ${PROJ_INSTALL_LIB_DIR})
install(DIRECTORY resources DESTINATION ${PROJ_INSTALL_BIN_DIR})
# --- install self!
install_tgt(pang)
