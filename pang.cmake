# executable : pang
# -- Version
set(pang_maj_ver 0)
set(pang_min_ver 0)
set(pang_pat_ver 1)

set(pang_lib_ver
    "${pang_maj_ver}.${pang_min_ver}.${pang_pat_ver}")

set(pang_lib_compat_ver
    "${pang_maj_ver}.${pang_min_ver}")

# -- Headers

# internal
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
add_comp_def(pang PANG_MAJ=${pang_maj_ver})
add_comp_def(pang PANG_MIN=${pang_min_ver})
add_comp_def(pang PANG_PAT=${pang_pat_ver})
add_link_flag(pang -L${sfml_dir}/lib)
link_libs(pang )
set_tgt_ver(pang ${pang_lib_ver} ${pang_lib_compat_ver})

# -- Install!
install(DIRECTORY ${sfml_dir}/lib/ DESTINATION ${PROJ_INSTALL_LIB_DIR})
install_tgt(pang)
