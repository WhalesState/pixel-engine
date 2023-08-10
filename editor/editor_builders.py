"""Functions used to generate source files during build time

All such functions are invoked in a subprocess on Windows to prevent build flakiness.

"""
import os
import os.path
import shutil
import subprocess
import tempfile
import uuid
import zlib
from platform_methods import subprocess_main


def make_doc_header(target, source, env):
    dst = target[0]
    g = open(dst, "w", encoding="utf-8")
    buf = ""
    docbegin = ""
    docend = ""
    for src in source:
        if not src.endswith(".xml"):
            continue
        with open(src, "r", encoding="utf-8") as f:
            content = f.read()
        buf += content

    buf = (docbegin + buf + docend).encode("utf-8")
    decomp_size = len(buf)

    # Use maximum zlib compression level to further reduce file size
    # (at the cost of initial build times).
    buf = zlib.compress(buf, zlib.Z_BEST_COMPRESSION)

    g.write("/* THIS FILE IS GENERATED DO NOT EDIT */\n")
    g.write("#ifndef _DOC_DATA_RAW_H\n")
    g.write("#define _DOC_DATA_RAW_H\n")
    g.write('static const char *_doc_data_hash = "' + str(hash(buf)) + '";\n')
    g.write("static const int _doc_data_compressed_size = " + str(len(buf)) + ";\n")
    g.write("static const int _doc_data_uncompressed_size = " + str(decomp_size) + ";\n")
    g.write("static const unsigned char _doc_data_compressed[] = {\n")
    for i in range(len(buf)):
        g.write("\t" + str(buf[i]) + ",\n")
    g.write("};\n")

    g.write("#endif")

    g.close()


def make_fonts_header(target, source, env):
    dst = target[0]

    g = open(dst, "w", encoding="utf-8")

    g.write("/* THIS FILE IS GENERATED DO NOT EDIT */\n")
    g.write("#ifndef _EDITOR_FONTS_H\n")
    g.write("#define _EDITOR_FONTS_H\n")

    # Saving uncompressed, since FreeType will reference from memory pointer.
    for i in range(len(source)):
        with open(source[i], "rb") as f:
            buf = f.read()

        name = os.path.splitext(os.path.basename(source[i]))[0]

        g.write("static const int _font_" + name + "_size = " + str(len(buf)) + ";\n")
        g.write("static const unsigned char _font_" + name + "[] = {\n")
        for j in range(len(buf)):
            g.write("\t" + str(buf[j]) + ",\n")

        g.write("};\n")

    g.write("#endif")

    g.close()


def make_translations_header(target, source, env, category):
    dst = target[0]

    g = open(dst, "w", encoding="utf-8")

    g.write("/* THIS FILE IS GENERATED DO NOT EDIT */\n")
    g.write("#ifndef _{}_TRANSLATIONS_H\n".format(category.upper()))
    g.write("#define _{}_TRANSLATIONS_H\n".format(category.upper()))

    sorted_paths = sorted(source, key=lambda path: os.path.splitext(os.path.basename(path))[0])

    msgfmt_available = shutil.which("msgfmt") is not None

    if not msgfmt_available:
        print("WARNING: msgfmt is not found, using .po files instead of .mo")

    xl_names = []
    for i in range(len(sorted_paths)):
        if msgfmt_available:
            mo_path = os.path.join(tempfile.gettempdir(), uuid.uuid4().hex + ".mo")
            cmd = "msgfmt " + sorted_paths[i] + " --no-hash -o " + mo_path
            try:
                subprocess.Popen(cmd, shell=True, stderr=subprocess.PIPE).communicate()
                with open(mo_path, "rb") as f:
                    buf = f.read()
            except OSError as e:
                print(
                    "WARNING: msgfmt execution failed, using .po file instead of .mo: path=%r; [%s] %s"
                    % (sorted_paths[i], e.__class__.__name__, e)
                )
                with open(sorted_paths[i], "rb") as f:
                    buf = f.read()
            finally:
                try:
                    os.remove(mo_path)
                except OSError as e:
                    # Do not fail the entire build if it cannot delete a temporary file
                    print(
                        "WARNING: Could not delete temporary .mo file: path=%r; [%s] %s"
                        % (mo_path, e.__class__.__name__, e)
                    )
        else:
            with open(sorted_paths[i], "rb") as f:
                buf = f.read()

        decomp_size = len(buf)
        # Use maximum zlib compression level to further reduce file size
        # (at the cost of initial build times).
        buf = zlib.compress(buf, zlib.Z_BEST_COMPRESSION)
        name = os.path.splitext(os.path.basename(sorted_paths[i]))[0]

        g.write("static const unsigned char _{}_translation_{}_compressed[] = {{\n".format(category, name))
        for j in range(len(buf)):
            g.write("\t" + str(buf[j]) + ",\n")

        g.write("};\n")

        xl_names.append([name, len(buf), str(decomp_size)])

    g.write("struct {}TranslationList {{\n".format(category.capitalize()))
    g.write("\tconst char* lang;\n")
    g.write("\tint comp_size;\n")
    g.write("\tint uncomp_size;\n")
    g.write("\tconst unsigned char* data;\n")
    g.write("};\n\n")
    g.write("static {}TranslationList _{}_translations[] = {{\n".format(category.capitalize(), category))
    for x in xl_names:
        g.write(
            '\t{{ "{}", {}, {}, _{}_translation_{}_compressed }},\n'.format(x[0], str(x[1]), str(x[2]), category, x[0])
        )
    g.write("\t{nullptr, 0, 0, nullptr}\n")
    g.write("};\n")

    g.write("#endif")

    g.close()


def make_editor_translations_header(target, source, env):
    make_translations_header(target, source, env, "editor")


def make_property_translations_header(target, source, env):
    make_translations_header(target, source, env, "property")


def make_doc_translations_header(target, source, env):
    make_translations_header(target, source, env, "doc")


if __name__ == "__main__":
    subprocess_main(globals())
