function(rf_emit_byte_array output_var input_file array_name)
    file(READ "${input_file}" shader_hex HEX)
    string(LENGTH "${shader_hex}" shader_hex_length)
    math(EXPR shader_byte_count "${shader_hex_length} / 2")

    set(array_body "")
    set(index 0)
    while(index LESS shader_hex_length)
        string(SUBSTRING "${shader_hex}" ${index} 2 byte_hex)
        string(APPEND array_body "0x${byte_hex}")
        math(EXPR index "${index} + 2")
        if(index LESS shader_hex_length)
            string(APPEND array_body ", ")
        endif()
    endwhile()

    set(${output_var} "alignas(4) inline constexpr std::array<std::uint8_t, ${shader_byte_count}> ${array_name} = { ${array_body} };" PARENT_SCOPE)
endfunction()

rf_emit_byte_array(VERT_ARRAY "${INPUT_VERT}" "kDebugTriangleVertSpv")
rf_emit_byte_array(FRAG_ARRAY "${INPUT_FRAG}" "kDebugTriangleFragSpv")

file(WRITE "${OUTPUT}" "#pragma once\n\n#include <array>\n#include <cstdint>\n\nnamespace roadforge::renderer::shaders {\n\n${VERT_ARRAY}\n${FRAG_ARRAY}\n\n} // namespace roadforge::renderer::shaders\n")
