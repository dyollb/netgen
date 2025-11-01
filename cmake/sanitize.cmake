# sanitize.cmake
# Cross-platform AddressSanitizer and UndefinedBehaviorSanitizer support for CMake

# Determine sanitizer support based on compiler and version
if(USE_ASAN AND NOT DEFINED NETGEN_SANITIZER_SUPPORT_CHECKED)
    set(NETGEN_SANITIZER_SUPPORT_CHECKED TRUE)
    
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        # Clang supports sanitizers since version 3.1 (AddressSanitizer) and 3.3 (UBSan)
        # Apple Clang and regular Clang both support these flags
        set(NETGEN_COMPILER_SUPPORTS_ASAN TRUE)
        set(NETGEN_COMPILER_SUPPORTS_UBSAN TRUE)
        message(STATUS "Clang compiler detected - enabling sanitizers")
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        # GCC supports AddressSanitizer since 4.8 and UBSan since 4.9
        if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "4.8")
            set(NETGEN_COMPILER_SUPPORTS_ASAN TRUE)
        else()
            set(NETGEN_COMPILER_SUPPORTS_ASAN FALSE)
        endif()
        
        if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "4.9")
            set(NETGEN_COMPILER_SUPPORTS_UBSAN TRUE)
        else()
            set(NETGEN_COMPILER_SUPPORTS_UBSAN FALSE)
        endif()
        
        message(STATUS "GCC ${CMAKE_CXX_COMPILER_VERSION} detected - AddressSanitizer: ${NETGEN_COMPILER_SUPPORTS_ASAN}, UndefinedBehaviorSanitizer: ${NETGEN_COMPILER_SUPPORTS_UBSAN}")
    else()
        # For other compilers, disable sanitizers
        set(NETGEN_COMPILER_SUPPORTS_ASAN FALSE)
        set(NETGEN_COMPILER_SUPPORTS_UBSAN FALSE)
        message(STATUS "Unknown compiler ${CMAKE_CXX_COMPILER_ID} - sanitizers disabled")
    endif()
endif()

# Macro to add sanitizer flags to a target
# Usage: add_sanitizers_to_target(<target_name>)
macro(add_sanitizers_to_target target_name)
    if(USE_ASAN)
        if(MSVC)
            # MSVC supports AddressSanitizer since VS 2019 16.9
            if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "19.28")
                target_compile_options(${target_name} PRIVATE /fsanitize=address)
                message(STATUS "Enabled AddressSanitizer for target '${target_name}' (MSVC)")
            else()
                message(WARNING "AddressSanitizer requires MSVC 2019 16.9 or later for target '${target_name}', current version: ${CMAKE_CXX_COMPILER_VERSION}")
            endif()
        elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            # GCC and Clang support both AddressSanitizer and UndefinedBehaviorSanitizer
            set(SANITIZER_FLAGS)
            
            # Use pre-checked compiler support
            if(NETGEN_COMPILER_SUPPORTS_ASAN)
                list(APPEND SANITIZER_FLAGS "-fsanitize=address")
            endif()
            
            if(NETGEN_COMPILER_SUPPORTS_UBSAN)
                list(APPEND SANITIZER_FLAGS "-fsanitize=undefined")
            endif()
            
            if(SANITIZER_FLAGS)
                target_compile_options(${target_name} PRIVATE ${SANITIZER_FLAGS})
                target_link_options(${target_name} PRIVATE ${SANITIZER_FLAGS})
                
                # Additional recommended flags for better debugging
                target_compile_options(${target_name} PRIVATE 
                    -fno-omit-frame-pointer 
                    -fno-optimize-sibling-calls
                )
                
                string(REPLACE ";" ", " SANITIZER_FLAGS_STR "${SANITIZER_FLAGS}")
                message(STATUS "Enabled sanitizers for target '${target_name}': ${SANITIZER_FLAGS_STR}")
            else()
                message(WARNING "No sanitizers available for target '${target_name}' with compiler: ${CMAKE_CXX_COMPILER_ID}")
            endif()
        else()
            message(WARNING "AddressSanitizer not supported for target '${target_name}' with compiler: ${CMAKE_CXX_COMPILER_ID}")
        endif()
    endif()
endmacro()

# Function to add sanitizers to multiple targets
# Usage: add_sanitizers_to_targets(<target1> <target2> ...)
function(add_sanitizers_to_targets)
    foreach(target_name ${ARGV})
        add_sanitizers_to_target(${target_name})
    endforeach()
endfunction()