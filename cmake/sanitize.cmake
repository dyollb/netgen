# sanitize.cmake
# Cross-platform AddressSanitizer, MemorySanitizer and UndefinedBehaviorSanitizer support for CMake

# Validate sanitizer options
if(USE_ASAN AND USE_MSAN)
    message(FATAL_ERROR "USE_ASAN and USE_MSAN cannot be used together - AddressSanitizer and MemorySanitizer are mutually exclusive")
endif()

# Early platform check for MemorySanitizer
if(USE_MSAN)
    if(NOT (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64"))
        message(FATAL_ERROR "MemorySanitizer (USE_MSAN) is only supported on Linux x86_64 platforms. Current platform: ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_PROCESSOR}")
    endif()
endif()

# Determine sanitizer support based on compiler and version
if((USE_ASAN OR USE_MSAN) AND NOT DEFINED NETGEN_SANITIZER_SUPPORT_CHECKED)
    set(NETGEN_SANITIZER_SUPPORT_CHECKED TRUE)
    
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        # Clang supports sanitizers since version 3.1 (AddressSanitizer), 3.3 (UBSan), and 3.3 (MemorySanitizer)
        # Apple Clang and regular Clang both support ASan and UBSan
        set(NETGEN_COMPILER_SUPPORTS_ASAN TRUE)
        set(NETGEN_COMPILER_SUPPORTS_UBSAN TRUE)
        
        # MemorySanitizer is only supported on Linux x86_64 with Clang
        if(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
            set(NETGEN_COMPILER_SUPPORTS_MSAN TRUE)
        else()
            set(NETGEN_COMPILER_SUPPORTS_MSAN FALSE)
        endif()
        
        message(STATUS "Clang compiler detected - AddressSanitizer: ${NETGEN_COMPILER_SUPPORTS_ASAN}, MemorySanitizer: ${NETGEN_COMPILER_SUPPORTS_MSAN}, UndefinedBehaviorSanitizer: ${NETGEN_COMPILER_SUPPORTS_UBSAN}")
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        # GCC supports AddressSanitizer since 4.8, UBSan since 4.9, and MemorySanitizer since 4.9
        if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "4.8")
            set(NETGEN_COMPILER_SUPPORTS_ASAN TRUE)
        else()
            set(NETGEN_COMPILER_SUPPORTS_ASAN FALSE)
        endif()
        
        if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "4.9")
            set(NETGEN_COMPILER_SUPPORTS_UBSAN TRUE)
            
            # MemorySanitizer is only supported on Linux x86_64 with GCC
            if(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
                set(NETGEN_COMPILER_SUPPORTS_MSAN TRUE)
            else()
                set(NETGEN_COMPILER_SUPPORTS_MSAN FALSE)
            endif()
        else()
            set(NETGEN_COMPILER_SUPPORTS_MSAN FALSE)
            set(NETGEN_COMPILER_SUPPORTS_UBSAN FALSE)
        endif()
        
        message(STATUS "GCC ${CMAKE_CXX_COMPILER_VERSION} detected - AddressSanitizer: ${NETGEN_COMPILER_SUPPORTS_ASAN}, MemorySanitizer: ${NETGEN_COMPILER_SUPPORTS_MSAN}, UndefinedBehaviorSanitizer: ${NETGEN_COMPILER_SUPPORTS_UBSAN}")
    else()
        # For other compilers, disable sanitizers
        set(NETGEN_COMPILER_SUPPORTS_ASAN FALSE)
        set(NETGEN_COMPILER_SUPPORTS_MSAN FALSE)
        set(NETGEN_COMPILER_SUPPORTS_UBSAN FALSE)
        message(STATUS "Unknown compiler ${CMAKE_CXX_COMPILER_ID} - sanitizers disabled")
    endif()
endif()

# Macro to add sanitizer flags to a target
# Usage: add_sanitizers_to_target(<target_name>)
macro(add_sanitizers_to_target target_name)
    if(USE_ASAN OR USE_MSAN)
        if(MSVC)
            # MSVC supports AddressSanitizer since VS 2019 16.9
            if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "19.28")
                target_compile_options(${target_name} PRIVATE /fsanitize=address)
                message(STATUS "Enabled AddressSanitizer for target '${target_name}' (MSVC)")
            else()
                message(WARNING "AddressSanitizer requires MSVC 2019 16.9 or later for target '${target_name}', current version: ${CMAKE_CXX_COMPILER_VERSION}")
            endif()
        elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            # GCC and Clang support AddressSanitizer, MemorySanitizer and UndefinedBehaviorSanitizer
            set(SANITIZER_FLAGS)
            
            # Use pre-checked compiler support
            if(USE_ASAN AND NETGEN_COMPILER_SUPPORTS_ASAN)
                list(APPEND SANITIZER_FLAGS "-fsanitize=address")
            endif()
            
            if(USE_MSAN AND NETGEN_COMPILER_SUPPORTS_MSAN)
                list(APPEND SANITIZER_FLAGS "-fsanitize=memory")
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
                
                # MSan requires special flags for better detection
                if(USE_MSAN AND NETGEN_COMPILER_SUPPORTS_MSAN)
                    target_compile_options(${target_name} PRIVATE 
                        -fsanitize-memory-track-origins=2
                        -fno-sanitize-recover=memory
                    )
                endif()
                
                string(REPLACE ";" ", " SANITIZER_FLAGS_STR "${SANITIZER_FLAGS}")
                message(STATUS "Enabled sanitizers for target '${target_name}': ${SANITIZER_FLAGS_STR}")
            else()
                message(WARNING "No sanitizers available for target '${target_name}' with compiler: ${CMAKE_CXX_COMPILER_ID}")
            endif()
        else()
            message(WARNING "Sanitizers not supported for target '${target_name}' with compiler: ${CMAKE_CXX_COMPILER_ID}")
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