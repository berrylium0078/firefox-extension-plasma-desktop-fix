# TODO refactor this AIGC shit

# Paths (relative to build dir)
set(EXTENSION_BUILD_DIR ${CMAKE_BINARY_DIR}/extension)
set(EXTENSION_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src/browser)
set(TS_CONFIG ${EXTENSION_SOURCE_DIR}/tsconfig.json)
set(TS_MAIN ${EXTENSION_SOURCE_DIR}/background.ts)
set(TS_SOURCE_FILES ${EXTENSION_SOURCE_DIR}/*.ts)
set(JS_OUTPUT ${EXTENSION_BUILD_DIR}/background.js)
set(MANIFEST_IN ${EXTENSION_SOURCE_DIR}/manifest.json.in)
set(MANIFST ${EXTENSION_BUILD_DIR}/manifest.json)

# Asset files to copy -- list them relative to source dir
set(ASSETS
    assets/icon.svg
    # Add other assets here
)

# Step 1: Syntax check with tsc, and bundle with npx
add_custom_command(
    OUTPUT ${JS_OUTPUT}
    COMMAND tsc --project ${TS_CONFIG} --noEmit && npx esbuild ${TS_MAIN} --bundle --outfile=${JS_OUTPUT}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/src/browser
    DEPENDS ${TS_SOURCE_FILES} ${TS_CONFIG}
    COMMENT "Compiling TypeScript background.ts"
    VERBATIM
)

# Step 2: generate manifest
configure_file(
    ${MANIFEST_IN}
    ${MANIFST}
    @ONLY
)

# Step 3: Copy asset files to build/extension directory

# We'll copy each asset individually, using a custom command per asset
set(COPIED_ASSETS)
foreach(ASSET IN LISTS ASSETS)
    get_filename_component(ASSET_NAME ${ASSET} NAME)
    set(ASSET_NAME ${CMAKE_SOURCE_DIR}/src/browser/${ASSET})
    set(DEST_FILE ${EXTENSION_BUILD_DIR}/${ASSET})

    add_custom_command(
        OUTPUT ${DEST_FILE}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${ASSET_NAME} ${DEST_FILE}
        DEPENDS ${ASSET_NAME}
        COMMENT "Copying asset ${ASSET_NAME}"
        VERBATIM
    )
    list(APPEND COPIED_ASSETS ${DEST_FILE})
endforeach()

# Step 4: Zip the extension directory to build/extension.xpi

set(XPI_OUTPUT ${CMAKE_BINARY_DIR}/extension.xpi)

add_custom_command(
    OUTPUT ${XPI_OUTPUT}
    COMMAND ${CMAKE_COMMAND} -E chdir ${EXTENSION_BUILD_DIR} 
            zip -FS -r ${XPI_OUTPUT} .
            DEPENDS ${JS_OUTPUT} ${COPIED_ASSETS} ${MANIFST}
    COMMENT "Packaging extension into extension.xpi"
    VERBATIM
)

# Define a custom target to build the full extension

add_custom_target(build_extension ALL
    DEPENDS ${XPI_OUTPUT}
    COMMENT "Build complete extension package"
)
