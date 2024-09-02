target_sources(${PROJECT_NAME} PRIVATE
    # {{BEGIN_TARGET_SOURCES}}
	${CMAKE_CURRENT_LIST_DIR}/src/fx_nand_flash_simulated_driver.c
	${CMAKE_CURRENT_LIST_DIR}/src/fx_nor_flash_simulator_driver.c
	${CMAKE_CURRENT_LIST_DIR}/src/lx_nand_flash_256byte_ecc_check.c
	${CMAKE_CURRENT_LIST_DIR}/src/lx_nand_flash_256byte_ecc_compute.c
	${CMAKE_CURRENT_LIST_DIR}/src/lx_nand_flash_block_full_update.c
	${CMAKE_CURRENT_LIST_DIR}/src/lx_nand_flash_block_obsoleted_check.c
	${CMAKE_CURRENT_LIST_DIR}/src/lx_nand_flash_block_reclaim.c
	${CMAKE_CURRENT_LIST_DIR}/src/lx_nand_flash_close.c
	${CMAKE_CURRENT_LIST_DIR}/src/lx_nand_flash_defragment.c
	${CMAKE_CURRENT_LIST_DIR}/src/lx_nand_flash_driver_block_erase.c
	${CMAKE_CURRENT_LIST_DIR}/src/lx_nand_flash_driver_block_erased_verify.c
	${CMAKE_CURRENT_LIST_DIR}/src/lx_nand_flash_driver_block_status_get.c
	${CMAKE_CURRENT_LIST_DIR}/src/lx_nand_flash_driver_block_status_set.c
	${CMAKE_CURRENT_LIST_DIR}/src/lx_nand_flash_driver_extra_bytes_get.c
	${CMAKE_CURRENT_LIST_DIR}/src/lx_nand_flash_driver_extra_bytes_set.c
	${CMAKE_CURRENT_LIST_DIR}/src/lx_nand_flash_driver_page_erased_verify.c
	${CMAKE_CURRENT_LIST_DIR}/src/lx_nand_flash_driver_read.c
	${CMAKE_CURRENT_LIST_DIR}/src/lx_nand_flash_driver_write.c
	${CMAKE_CURRENT_LIST_DIR}/src/lx_nand_flash_extended_cache_enable.c
	${CMAKE_CURRENT_LIST_DIR}/src/lx_nand_flash_initialize.c
	${CMAKE_CURRENT_LIST_DIR}/src/lx_nand_flash_logical_sector_find.c
	${CMAKE_CURRENT_LIST_DIR}/src/lx_nand_flash_next_block_to_erase_find.c
	${CMAKE_CURRENT_LIST_DIR}/src/lx_nand_flash_open.c
	${CMAKE_CURRENT_LIST_DIR}/src/lx_nand_flash_page_ecc_check.c
	${CMAKE_CURRENT_LIST_DIR}/src/lx_nand_flash_page_ecc_compute.c
	${CMAKE_CURRENT_LIST_DIR}/src/lx_nand_flash_partial_defragment.c
	${CMAKE_CURRENT_LIST_DIR}/src/lx_nand_flash_physical_page_allocate.c
	${CMAKE_CURRENT_LIST_DIR}/src/lx_nand_flash_sector_mapping_cache_invalidate.c
	${CMAKE_CURRENT_LIST_DIR}/src/lx_nand_flash_sector_read.c
	${CMAKE_CURRENT_LIST_DIR}/src/lx_nand_flash_sector_release.c
	${CMAKE_CURRENT_LIST_DIR}/src/lx_nand_flash_sector_write.c
	${CMAKE_CURRENT_LIST_DIR}/src/lx_nand_flash_simulator.c
	${CMAKE_CURRENT_LIST_DIR}/src/lx_nand_flash_system_error.c
	${CMAKE_CURRENT_LIST_DIR}/src/lx_nor_flash_block_reclaim.c
	${CMAKE_CURRENT_LIST_DIR}/src/lx_nor_flash_close.c
	${CMAKE_CURRENT_LIST_DIR}/src/lx_nor_flash_defragment.c
	${CMAKE_CURRENT_LIST_DIR}/src/lx_nor_flash_driver_block_erase.c
	${CMAKE_CURRENT_LIST_DIR}/src/lx_nor_flash_driver_read.c
	${CMAKE_CURRENT_LIST_DIR}/src/lx_nor_flash_driver_write.c
	${CMAKE_CURRENT_LIST_DIR}/src/lx_nor_flash_extended_cache_enable.c
	${CMAKE_CURRENT_LIST_DIR}/src/lx_nor_flash_initialize.c
	${CMAKE_CURRENT_LIST_DIR}/src/lx_nor_flash_logical_sector_find.c
	${CMAKE_CURRENT_LIST_DIR}/src/lx_nor_flash_next_block_to_erase_find.c
	${CMAKE_CURRENT_LIST_DIR}/src/lx_nor_flash_open.c
	${CMAKE_CURRENT_LIST_DIR}/src/lx_nor_flash_partial_defragment.c
	${CMAKE_CURRENT_LIST_DIR}/src/lx_nor_flash_physical_sector_allocate.c
	${CMAKE_CURRENT_LIST_DIR}/src/lx_nor_flash_sector_mapping_cache_invalidate.c
	${CMAKE_CURRENT_LIST_DIR}/src/lx_nor_flash_sector_read.c
	${CMAKE_CURRENT_LIST_DIR}/src/lx_nor_flash_sector_release.c
	${CMAKE_CURRENT_LIST_DIR}/src/lx_nor_flash_sector_write.c
	${CMAKE_CURRENT_LIST_DIR}/src/lx_nor_flash_simulator.c
	${CMAKE_CURRENT_LIST_DIR}/src/lx_nor_flash_system_error.c

    # {{END_TARGET_SOURCES}}
)

# Add the Common/inc directory to the project include list
target_include_directories(${PROJECT_NAME} PUBLIC 
    ${CMAKE_CURRENT_LIST_DIR}/inc
)

