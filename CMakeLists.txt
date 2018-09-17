cmake_minimum_required(VERSION 3.8)
project(Liblightnvm_test)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        addr_test.c
        block_erase_unit.c
        erase_rw_crash.c
        get_bbt.c
        parallel_nvm_write.c
        parallel_plane.c
        read_hw.c
        rw_unit_test.c
        scan_erase_badblock.c
        sec_unit_writ.c
        set_badblk.c
        showinfo.c
        write_hw.c
        wrong_addr_test.c)

add_executable(Liblightnvm_test ${SOURCE_FILES})