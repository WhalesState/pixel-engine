/**************************************************************************/
/*  editor_asset_installer.cpp                                            */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "editor_asset_installer.h"

#include "core/io/dir_access.h"
#include "core/io/file_access.h"
#include "core/io/zip_io.h"
#include "editor/editor_file_system.h"
#include "editor/editor_node.h"
#include "editor/progress_dialog.h"

void EditorAssetInstaller::_item_edited() {
	if (updating) {
		return;
	}

	TreeItem *item = tree->get_edited();
	if (!item) {
		return;
	}

	updating = true;
	item->propagate_check(0);
	updating = false;
}

void EditorAssetInstaller::_check_propagated_to_item(Object *p_obj, int column) {
	TreeItem *affected_item = Object::cast_to<TreeItem>(p_obj);
	if (affected_item && affected_item->get_custom_color(0) != Color()) {
		affected_item->set_checked(0, false);
		affected_item->propagate_check(0, false);
	}
}

void EditorAssetInstaller::open(const String &p_path, int p_depth) {
	package_path = p_path;
	HashSet<String> files_sorted;

	Ref<FileAccess> io_fa;
	zlib_filefunc_def io = zipio_create_io(&io_fa);

	unzFile pkg = unzOpen2(p_path.utf8().get_data(), &io);
	if (!pkg) {
		error->set_text(vformat(TTR("Error opening asset file for \"%s\" (not in ZIP format)."), asset_name));
		return;
	}

	int ret = unzGoToFirstFile(pkg);

	while (ret == UNZ_OK) {
		//get filename
		unz_file_info info;
		char fname[16384];
		unzGetCurrentFileInfo(pkg, &info, fname, 16384, nullptr, 0, nullptr, 0);

		String name = String::utf8(fname);
		files_sorted.insert(name);

		ret = unzGoToNextFile(pkg);
	}

	HashMap<String, Ref<Texture2D>> extension_guess;
	{
		extension_guess["bmp"] = tree->get_theme_icon(SNAME("ImageTexture"), SNAME("EditorIcons"));
		extension_guess["dds"] = tree->get_theme_icon(SNAME("ImageTexture"), SNAME("EditorIcons"));
		extension_guess["exr"] = tree->get_theme_icon(SNAME("ImageTexture"), SNAME("EditorIcons"));
		extension_guess["jpg"] = tree->get_theme_icon(SNAME("ImageTexture"), SNAME("EditorIcons"));
		extension_guess["jpeg"] = tree->get_theme_icon(SNAME("ImageTexture"), SNAME("EditorIcons"));
		extension_guess["png"] = tree->get_theme_icon(SNAME("ImageTexture"), SNAME("EditorIcons"));
		extension_guess["svg"] = tree->get_theme_icon(SNAME("ImageTexture"), SNAME("EditorIcons"));
		extension_guess["tga"] = tree->get_theme_icon(SNAME("ImageTexture"), SNAME("EditorIcons"));
		extension_guess["webp"] = tree->get_theme_icon(SNAME("ImageTexture"), SNAME("EditorIcons"));

		extension_guess["wav"] = tree->get_theme_icon(SNAME("AudioStreamWAV"), SNAME("EditorIcons"));
		extension_guess["ogg"] = tree->get_theme_icon(SNAME("AudioStreamOggVorbis"), SNAME("EditorIcons"));
		extension_guess["mp3"] = tree->get_theme_icon(SNAME("AudioStreamMP3"), SNAME("EditorIcons"));

		extension_guess["scn"] = tree->get_theme_icon(SNAME("PackedScene"), SNAME("EditorIcons"));
		extension_guess["tscn"] = tree->get_theme_icon(SNAME("PackedScene"), SNAME("EditorIcons"));
		extension_guess["escn"] = tree->get_theme_icon(SNAME("PackedScene"), SNAME("EditorIcons"));
		extension_guess["dae"] = tree->get_theme_icon(SNAME("PackedScene"), SNAME("EditorIcons"));
		extension_guess["glb"] = tree->get_theme_icon(SNAME("PackedScene"), SNAME("EditorIcons"));

		extension_guess["gdshader"] = tree->get_theme_icon(SNAME("Shader"), SNAME("EditorIcons"));
		extension_guess["gdshaderinc"] = tree->get_theme_icon(SNAME("TextFile"), SNAME("EditorIcons"));
		extension_guess["gd"] = tree->get_theme_icon(SNAME("GDScript"), SNAME("EditorIcons"));
		if (Engine::get_singleton()->has_singleton("GodotSharp")) {
			extension_guess["cs"] = tree->get_theme_icon(SNAME("CSharpScript"), SNAME("EditorIcons"));
		} else {
			// Mark C# support as unavailable.
			extension_guess["cs"] = tree->get_theme_icon(SNAME("ImportFail"), SNAME("EditorIcons"));
		}

		extension_guess["res"] = tree->get_theme_icon(SNAME("Resource"), SNAME("EditorIcons"));
		extension_guess["tres"] = tree->get_theme_icon(SNAME("Resource"), SNAME("EditorIcons"));
		extension_guess["atlastex"] = tree->get_theme_icon(SNAME("AtlasTexture"), SNAME("EditorIcons"));
		// By default, OBJ files are imported as Mesh resources rather than PackedScenes.
		extension_guess["obj"] = tree->get_theme_icon(SNAME("Mesh"), SNAME("EditorIcons"));

		extension_guess["txt"] = tree->get_theme_icon(SNAME("TextFile"), SNAME("EditorIcons"));
		extension_guess["md"] = tree->get_theme_icon(SNAME("TextFile"), SNAME("EditorIcons"));
		extension_guess["rst"] = tree->get_theme_icon(SNAME("TextFile"), SNAME("EditorIcons"));
		extension_guess["json"] = tree->get_theme_icon(SNAME("TextFile"), SNAME("EditorIcons"));
		extension_guess["yml"] = tree->get_theme_icon(SNAME("TextFile"), SNAME("EditorIcons"));
		extension_guess["yaml"] = tree->get_theme_icon(SNAME("TextFile"), SNAME("EditorIcons"));
		extension_guess["toml"] = tree->get_theme_icon(SNAME("TextFile"), SNAME("EditorIcons"));
		extension_guess["cfg"] = tree->get_theme_icon(SNAME("TextFile"), SNAME("EditorIcons"));
		extension_guess["ini"] = tree->get_theme_icon(SNAME("TextFile"), SNAME("EditorIcons"));
	}

	Ref<Texture2D> generic_extension = tree->get_theme_icon(SNAME("Object"), SNAME("EditorIcons"));

	unzClose(pkg);

	updating = true;
	tree->clear();
	TreeItem *root = tree->create_item();
	root->set_cell_mode(0, TreeItem::CELL_MODE_CHECK);
	root->set_checked(0, true);
	root->set_icon(0, tree->get_theme_icon(SNAME("folder"), SNAME("FileDialog")));
	root->set_text(0, "res://");
	root->set_editable(0, true);
	HashMap<String, TreeItem *> dir_map;

	int num_file_conflicts = 0;

	for (const String &E : files_sorted) {
		String path = E;
		int depth = p_depth;
		bool skip = false;
		while (depth > 0) {
			int pp = path.find("/");
			if (pp == -1) {
				skip = true;
				break;
			}
			path = path.substr(pp + 1, path.length());
			depth--;
		}

		if (skip || path.is_empty()) {
			continue;
		}

		bool isdir = false;

		if (path.ends_with("/")) {
			//a directory
			path = path.substr(0, path.length() - 1);
			isdir = true;
		}

		int pp = path.rfind("/");

		TreeItem *parent_item;
		if (pp == -1) {
			parent_item = root;
		} else {
			String ppath = path.substr(0, pp);
			ERR_CONTINUE(!dir_map.has(ppath));
			parent_item = dir_map[ppath];
		}

		TreeItem *ti = tree->create_item(parent_item);
		ti->set_cell_mode(0, TreeItem::CELL_MODE_CHECK);
		ti->set_checked(0, true);
		ti->set_editable(0, true);
		if (isdir) {
			dir_map[path] = ti;
			ti->set_text(0, path.get_file() + "/");
			ti->set_icon(0, tree->get_theme_icon(SNAME("folder"), SNAME("FileDialog")));
			ti->set_metadata(0, String());
		} else {
			String file = path.get_file();
			String extension = file.get_extension().to_lower();
			if (extension_guess.has(extension)) {
				ti->set_icon(0, extension_guess[extension]);
			} else {
				ti->set_icon(0, generic_extension);
			}
			ti->set_text(0, file);

			String res_path = "res://" + path;
			if (FileAccess::exists(res_path)) {
				num_file_conflicts += 1;
				ti->set_custom_color(0, tree->get_theme_color(SNAME("error_color"), SNAME("Editor")));
				ti->set_tooltip_text(0, vformat(TTR("%s (already exists)"), res_path));
				ti->set_checked(0, false);
				ti->propagate_check(0);
			} else {
				ti->set_tooltip_text(0, res_path);
			}

			ti->set_metadata(0, res_path);
		}

		status_map[E] = ti;
	}

	if (num_file_conflicts >= 1) {
		asset_contents->set_text(vformat(TTR("Contents of asset \"%s\" - %d file(s) conflict with your project:"), asset_name, num_file_conflicts));
	} else {
		asset_contents->set_text(vformat(TTR("Contents of asset \"%s\" - No files conflict with your project:"), asset_name));
	}

	popup_centered_ratio(0.5);
	updating = false;
}

void EditorAssetInstaller::ok_pressed() {
	Ref<FileAccess> io_fa;
	zlib_filefunc_def io = zipio_create_io(&io_fa);

	unzFile pkg = unzOpen2(package_path.utf8().get_data(), &io);
	if (!pkg) {
		error->set_text(vformat(TTR("Error opening asset file for \"%s\" (not in ZIP format)."), asset_name));
		return;
	}

	int ret = unzGoToFirstFile(pkg);

	Vector<String> failed_files;

	ProgressDialog::get_singleton()->add_task("uncompress", TTR("Uncompressing Assets"), status_map.size());

	int idx = 0;
	while (ret == UNZ_OK) {
		//get filename
		unz_file_info info;
		char fname[16384];
		ret = unzGetCurrentFileInfo(pkg, &info, fname, 16384, nullptr, 0, nullptr, 0);
		if (ret != UNZ_OK) {
			break;
		}

		String name = String::utf8(fname);

		if (status_map.has(name) && (status_map[name]->is_checked(0) || status_map[name]->is_indeterminate(0))) {
			String path = status_map[name]->get_metadata(0);
			if (path.is_empty()) { // a dir

				String dirpath;
				TreeItem *t = status_map[name];
				while (t) {
					dirpath = t->get_text(0) + dirpath;
					t = t->get_parent();
				}

				if (dirpath.ends_with("/")) {
					dirpath = dirpath.substr(0, dirpath.length() - 1);
				}

				Ref<DirAccess> da = DirAccess::create(DirAccess::ACCESS_RESOURCES);
				da->make_dir(dirpath);
			} else {
				Vector<uint8_t> uncomp_data;
				uncomp_data.resize(info.uncompressed_size);

				//read
				unzOpenCurrentFile(pkg);
				unzReadCurrentFile(pkg, uncomp_data.ptrw(), uncomp_data.size());
				unzCloseCurrentFile(pkg);

				Ref<FileAccess> f = FileAccess::open(path, FileAccess::WRITE);
				if (f.is_valid()) {
					f->store_buffer(uncomp_data.ptr(), uncomp_data.size());
				} else {
					failed_files.push_back(path);
				}

				ProgressDialog::get_singleton()->task_step("uncompress", path, idx);
			}
		}

		idx++;
		ret = unzGoToNextFile(pkg);
	}

	ProgressDialog::get_singleton()->end_task("uncompress");
	unzClose(pkg);

	if (failed_files.size()) {
		String msg = vformat(TTR("The following files failed extraction from asset \"%s\":"), asset_name) + "\n\n";
		for (int i = 0; i < failed_files.size(); i++) {
			if (i > 15) {
				msg += "\n" + vformat(TTR("(and %s more files)"), itos(failed_files.size() - i));
				break;
			}
			msg += failed_files[i];
		}
		if (EditorNode::get_singleton() != nullptr) {
			EditorNode::get_singleton()->show_warning(msg);
		}
	} else {
		if (EditorNode::get_singleton() != nullptr) {
			EditorNode::get_singleton()->show_warning(vformat(TTR("Asset \"%s\" installed successfully!"), asset_name), TTR("Success!"));
		}
	}
	EditorFileSystem::get_singleton()->scan_changes();
}

void EditorAssetInstaller::set_asset_name(const String &p_asset_name) {
	asset_name = p_asset_name;
}

String EditorAssetInstaller::get_asset_name() const {
	return asset_name;
}

void EditorAssetInstaller::_bind_methods() {
}

EditorAssetInstaller::EditorAssetInstaller() {
	VBoxContainer *vb = memnew(VBoxContainer);
	add_child(vb);

	asset_contents = memnew(Label);
	vb->add_child(asset_contents);

	tree = memnew(Tree);
	tree->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	tree->connect("item_edited", callable_mp(this, &EditorAssetInstaller::_item_edited));
	tree->connect("check_propagated_to_item", callable_mp(this, &EditorAssetInstaller::_check_propagated_to_item));
	vb->add_child(tree);

	error = memnew(AcceptDialog);
	add_child(error);
	set_ok_button_text(TTR("Install"));
	set_title(TTR("Asset Installer"));

	set_hide_on_ok(true);
}
