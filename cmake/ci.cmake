if(DEFINED CI_JOB_INDEX AND DEFINED CI_JOB_COUNT)
    math(EXPR job_name "${CI_JOB_INDEX}+1")
    message("-- configuring CI job ${job_name}/${CI_JOB_COUNT}")
    get_property(targets GLOBAL PROPERTY RELEASE_TARGETS)
    list(LENGTH targets count)
    math(EXPR per_job "(${count}+${CI_JOB_COUNT}-1)/${CI_JOB_COUNT}")
    message("-- ${per_job} targets per job")
    math(EXPR start "${CI_JOB_INDEX}*${per_job}")
    list(SUBLIST targets ${start} ${per_job} ci_targets)
    message("-- will build targets: ${ci_targets}")
    add_custom_target(ci
        ${CMAKE_COMMAND} -E true
        DEPENDS ${ci_targets}
)
endif()
