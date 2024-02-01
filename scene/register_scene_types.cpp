/**************************************************************************/
/*  register_scene_types.cpp                                              */
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

#include "register_scene_types.h"

#include "core/config/project_settings.h"
#include "core/extension/gdextension_manager.h"
#include "core/object/class_db.h"
#include "core/os/os.h"
#include "scene/2d/animated_sprite_2d.h"
#include "scene/2d/area_2d.h"
#include "scene/2d/audio_listener_2d.h"
#include "scene/2d/audio_stream_player_2d.h"
#include "scene/2d/back_buffer_copy.h"
#include "scene/2d/camera_2d.h"
#include "scene/2d/canvas_group.h"
#include "scene/2d/canvas_modulate.h"
#include "scene/2d/collision_polygon_2d.h"
#include "scene/2d/collision_shape_2d.h"
#include "scene/2d/cpu_particles_2d.h"
#include "scene/2d/gpu_particles_2d.h"
#include "scene/2d/joint_2d.h"
#include "scene/2d/light_2d.h"
#include "scene/2d/light_occluder_2d.h"
#include "scene/2d/line_2d.h"
#include "scene/2d/marker_2d.h"
#include "scene/2d/parallax_background.h"
#include "scene/2d/parallax_layer.h"
#include "scene/2d/path_2d.h"
#include "scene/2d/physical_bone_2d.h"
#include "scene/2d/physics_body_2d.h"
#include "scene/2d/polygon_2d.h"
#include "scene/2d/ray_cast_2d.h"
#include "scene/2d/remote_transform_2d.h"
#include "scene/2d/shape_cast_2d.h"
#include "scene/2d/skeleton_2d.h"
#include "scene/2d/sprite_2d.h"
#include "scene/2d/tile_map.h"
#include "scene/2d/touch_screen_button.h"
#include "scene/2d/visible_on_screen_notifier_2d.h"
#include "scene/animation/animation_blend_space_1d.h"
#include "scene/animation/animation_blend_space_2d.h"
#include "scene/animation/animation_blend_tree.h"
#include "scene/animation/animation_mixer.h"
#include "scene/animation/animation_node_state_machine.h"
#include "scene/animation/animation_player.h"
#include "scene/animation/animation_tree.h"
#include "scene/animation/tween.h"
#include "scene/audio/audio_stream_player.h"
#include "scene/debugger/scene_debugger.h"
#include "scene/gui/aspect_ratio_container.h"
#include "scene/gui/box_container.h"
#include "scene/gui/button.h"
#include "scene/gui/center_container.h"
#include "scene/gui/check_box.h"
#include "scene/gui/check_button.h"
#include "scene/gui/code_edit.h"
#include "scene/gui/color_picker.h"
#include "scene/gui/color_rect.h"
#include "scene/gui/control.h"
#include "scene/gui/dialogs.h"
#include "scene/gui/file_dialog.h"
#include "scene/gui/flow_container.h"
#include "scene/gui/foldable_container.h"
#include "scene/gui/graph_edit.h"
#include "scene/gui/graph_node.h"
#include "scene/gui/grid_container.h"
#include "scene/gui/item_list.h"
#include "scene/gui/label.h"
#include "scene/gui/line_edit.h"
#include "scene/gui/link_button.h"
#include "scene/gui/margin_container.h"
#include "scene/gui/menu_bar.h"
#include "scene/gui/menu_button.h"
#include "scene/gui/nine_patch_rect.h"
#include "scene/gui/option_button.h"
#include "scene/gui/panel.h"
#include "scene/gui/panel_container.h"
#include "scene/gui/popup_menu.h"
#include "scene/gui/progress_bar.h"
#include "scene/gui/reference_rect.h"
#include "scene/gui/rich_text_effect.h"
#include "scene/gui/rich_text_label.h"
#include "scene/gui/scroll_bar.h"
#include "scene/gui/scroll_container.h"
#include "scene/gui/separator.h"
#include "scene/gui/slider.h"
#include "scene/gui/spin_box.h"
#include "scene/gui/split_container.h"
#include "scene/gui/subviewport_container.h"
#include "scene/gui/tab_bar.h"
#include "scene/gui/tab_container.h"
#include "scene/gui/text_edit.h"
#include "scene/gui/texture_button.h"
#include "scene/gui/texture_progress_bar.h"
#include "scene/gui/texture_rect.h"
#include "scene/gui/tree.h"
#include "scene/gui/video_stream_player.h"
#include "scene/main/canvas_item.h"
#include "scene/main/canvas_layer.h"
#include "scene/main/http_request.h"
#include "scene/main/instance_placeholder.h"
#include "scene/main/missing_node.h"
#include "scene/main/multiplayer_api.h"
#include "scene/main/resource_preloader.h"
#include "scene/main/scene_tree.h"
#include "scene/main/timer.h"
#include "scene/main/viewport.h"
#include "scene/main/window.h"
#include "scene/resources/animated_texture.h"
#include "scene/resources/animation_library.h"
#include "scene/resources/atlas_texture.h"
#include "scene/resources/audio_stream_polyphonic.h"
#include "scene/resources/audio_stream_wav.h"
#include "scene/resources/bit_map.h"
#include "scene/resources/camera_texture.h"
#include "scene/resources/capsule_shape_2d.h"
#include "scene/resources/circle_shape_2d.h"
#include "scene/resources/compressed_texture.h"
#include "scene/resources/concave_polygon_shape_2d.h"
#include "scene/resources/convex_polygon_shape_2d.h"
#include "scene/resources/font.h"
#include "scene/resources/gradient.h"
#include "scene/resources/gradient_texture.h"
#include "scene/resources/image_texture.h"
#include "scene/resources/label_settings.h"
#include "scene/resources/material.h"
#include "scene/resources/mesh_data_tool.h"
#include "scene/resources/multimesh.h"
#include "scene/resources/packed_scene.h"
#include "scene/resources/particle_process_material.h"
#include "scene/resources/physics_material.h"
#include "scene/resources/placeholder_textures.h"
#include "scene/resources/polygon_path_finder.h"
#include "scene/resources/portable_compressed_texture.h"
#include "scene/resources/rectangle_shape_2d.h"
#include "scene/resources/resource_format_text.h"
#include "scene/resources/segment_shape_2d.h"
#include "scene/resources/separation_ray_shape_2d.h"
#include "scene/resources/shader_include.h"
#include "scene/resources/skeleton_modification_2d.h"
#include "scene/resources/skeleton_modification_2d_ccdik.h"
#include "scene/resources/skeleton_modification_2d_fabrik.h"
#include "scene/resources/skeleton_modification_2d_jiggle.h"
#include "scene/resources/skeleton_modification_2d_lookat.h"
#include "scene/resources/skeleton_modification_2d_physicalbones.h"
#include "scene/resources/skeleton_modification_2d_stackholder.h"
#include "scene/resources/skeleton_modification_2d_twoboneik.h"
#include "scene/resources/skeleton_modification_stack_2d.h"
#include "scene/resources/style_box.h"
#include "scene/resources/style_box_flat.h"
#include "scene/resources/style_box_line.h"
#include "scene/resources/style_box_texture.h"
#include "scene/resources/syntax_highlighter.h"
#include "scene/resources/text_file.h"
#include "scene/resources/text_line.h"
#include "scene/resources/text_paragraph.h"
#include "scene/resources/texture.h"
#include "scene/resources/texture_rd.h"
#include "scene/resources/theme.h"
#include "scene/resources/tile_set.h"
#include "scene/resources/video_stream.h"
#include "scene/resources/visual_shader.h"
#include "scene/resources/visual_shader_nodes.h"
#include "scene/resources/visual_shader_particle_nodes.h"
#include "scene/resources/visual_shader_sdf_nodes.h"
#include "scene/resources/world_2d.h"
#include "scene/resources/world_boundary_shape_2d.h"
#include "scene/scene_string_names.h"
#include "scene/theme/theme_db.h"

#include "scene/main/shader_globals_override.h"

static Ref<ResourceFormatSaverText> resource_saver_text;
static Ref<ResourceFormatLoaderText> resource_loader_text;

static Ref<ResourceFormatLoaderCompressedTexture2D> resource_loader_stream_texture;
static Ref<ResourceFormatLoaderCompressedTextureLayered> resource_loader_texture_layered;
static Ref<ResourceFormatLoaderCompressedTexture3D> resource_loader_texture_3d;

static Ref<ResourceFormatSaverShader> resource_saver_shader;
static Ref<ResourceFormatLoaderShader> resource_loader_shader;

static Ref<ResourceFormatSaverShaderInclude> resource_saver_shader_include;
static Ref<ResourceFormatLoaderShaderInclude> resource_loader_shader_include;

void register_scene_types() {
	SceneStringNames::create();

	OS::get_singleton()->yield(); // may take time to init

	Node::init_node_hrcr();

	resource_loader_stream_texture.instantiate();
	ResourceLoader::add_resource_format_loader(resource_loader_stream_texture);

	resource_loader_texture_layered.instantiate();
	ResourceLoader::add_resource_format_loader(resource_loader_texture_layered);

	resource_loader_texture_3d.instantiate();
	ResourceLoader::add_resource_format_loader(resource_loader_texture_3d);

	resource_saver_text.instantiate();
	ResourceSaver::add_resource_format_saver(resource_saver_text, true);

	resource_loader_text.instantiate();
	ResourceLoader::add_resource_format_loader(resource_loader_text, true);

	resource_saver_shader.instantiate();
	ResourceSaver::add_resource_format_saver(resource_saver_shader, true);

	resource_loader_shader.instantiate();
	ResourceLoader::add_resource_format_loader(resource_loader_shader, true);

	resource_saver_shader_include.instantiate();
	ResourceSaver::add_resource_format_saver(resource_saver_shader_include, true);

	resource_loader_shader_include.instantiate();
	ResourceLoader::add_resource_format_loader(resource_loader_shader_include, true);

	OS::get_singleton()->yield(); // may take time to init

	GDREGISTER_CLASS(Object);

	GDREGISTER_CLASS(Node);
	GDREGISTER_VIRTUAL_CLASS(MissingNode);
	GDREGISTER_ABSTRACT_CLASS(InstancePlaceholder);

	GDREGISTER_ABSTRACT_CLASS(Viewport);
	GDREGISTER_CLASS(SubViewport);
	GDREGISTER_CLASS(ViewportTexture);

	GDREGISTER_ABSTRACT_CLASS(MultiplayerPeer);
	GDREGISTER_CLASS(MultiplayerPeerExtension);
	GDREGISTER_ABSTRACT_CLASS(MultiplayerAPI);
	GDREGISTER_CLASS(MultiplayerAPIExtension);

	GDREGISTER_CLASS(HTTPRequest);
	GDREGISTER_CLASS(Timer);
	GDREGISTER_CLASS(CanvasLayer);
	GDREGISTER_CLASS(CanvasModulate);
	GDREGISTER_CLASS(ResourcePreloader);
	GDREGISTER_CLASS(Window);

	/* REGISTER GUI */

	GDREGISTER_CLASS(ButtonGroup);
	GDREGISTER_VIRTUAL_CLASS(BaseButton);

	OS::get_singleton()->yield(); // may take time to init

	GDREGISTER_CLASS(Control);
	GDREGISTER_CLASS(Button);
	GDREGISTER_CLASS(Label);
	GDREGISTER_ABSTRACT_CLASS(ScrollBar);
	GDREGISTER_CLASS(HScrollBar);
	GDREGISTER_CLASS(VScrollBar);
	GDREGISTER_CLASS(ProgressBar);
	GDREGISTER_ABSTRACT_CLASS(Slider);
	GDREGISTER_CLASS(HSlider);
	GDREGISTER_CLASS(VSlider);
	GDREGISTER_CLASS(Popup);
	GDREGISTER_CLASS(PopupPanel);
	GDREGISTER_CLASS(MenuBar);
	GDREGISTER_CLASS(MenuButton);
	GDREGISTER_CLASS(CheckBox);
	GDREGISTER_CLASS(CheckButton);
	GDREGISTER_CLASS(LinkButton);
	GDREGISTER_CLASS(Panel);
	GDREGISTER_VIRTUAL_CLASS(Range);

	OS::get_singleton()->yield(); // may take time to init

	GDREGISTER_CLASS(TextureRect);
	GDREGISTER_CLASS(ColorRect);
	GDREGISTER_CLASS(NinePatchRect);
	GDREGISTER_CLASS(ReferenceRect);
	GDREGISTER_CLASS(AspectRatioContainer);
	GDREGISTER_CLASS(TabContainer);
	GDREGISTER_CLASS(TabBar);
	GDREGISTER_ABSTRACT_CLASS(Separator);
	GDREGISTER_CLASS(HSeparator);
	GDREGISTER_CLASS(VSeparator);
	GDREGISTER_CLASS(TextureButton);
	GDREGISTER_CLASS(Container);
	GDREGISTER_CLASS(BoxContainer);
	GDREGISTER_CLASS(HBoxContainer);
	GDREGISTER_CLASS(VBoxContainer);
	GDREGISTER_CLASS(GridContainer);
	GDREGISTER_CLASS(CenterContainer);
	GDREGISTER_CLASS(ScrollContainer);
	GDREGISTER_CLASS(PanelContainer);
	GDREGISTER_CLASS(FoldableContainer);
	GDREGISTER_CLASS(FlowContainer);
	GDREGISTER_CLASS(HFlowContainer);
	GDREGISTER_CLASS(VFlowContainer);
	GDREGISTER_CLASS(MarginContainer);

	OS::get_singleton()->yield(); // may take time to init

	GDREGISTER_CLASS(TextureProgressBar);
	GDREGISTER_CLASS(ItemList);

	GDREGISTER_CLASS(LineEdit);
	GDREGISTER_CLASS(VideoStreamPlayer);
	GDREGISTER_VIRTUAL_CLASS(VideoStreamPlayback);
	GDREGISTER_VIRTUAL_CLASS(VideoStream);

#ifndef ADVANCED_GUI_DISABLED
	GDREGISTER_CLASS(FileDialog);

	GDREGISTER_CLASS(PopupMenu);
	GDREGISTER_CLASS(Tree);

	GDREGISTER_CLASS(TextEdit);
	GDREGISTER_CLASS(CodeEdit);
	GDREGISTER_CLASS(SyntaxHighlighter);
	GDREGISTER_CLASS(CodeHighlighter);

	GDREGISTER_ABSTRACT_CLASS(TreeItem);
	GDREGISTER_CLASS(OptionButton);
	GDREGISTER_CLASS(SpinBox);
	GDREGISTER_CLASS(ColorPicker);
	GDREGISTER_CLASS(ColorPickerButton);
	GDREGISTER_CLASS(RichTextLabel);
	GDREGISTER_CLASS(RichTextEffect);
	GDREGISTER_CLASS(CharFXTransform);

	GDREGISTER_CLASS(AcceptDialog);
	GDREGISTER_CLASS(ConfirmationDialog);

	GDREGISTER_CLASS(SubViewportContainer);
	GDREGISTER_CLASS(SplitContainer);
	GDREGISTER_CLASS(HSplitContainer);
	GDREGISTER_CLASS(VSplitContainer);

	GDREGISTER_CLASS(GraphElement);
	GDREGISTER_CLASS(GraphNode);
	GDREGISTER_CLASS(GraphEdit);

	OS::get_singleton()->yield(); // may take time to init

	bool swap_cancel_ok = false;
	if (DisplayServer::get_singleton()) {
		swap_cancel_ok = GLOBAL_DEF_NOVAL("gui/common/swap_cancel_ok", bool(DisplayServer::get_singleton()->get_swap_cancel_ok()));
	}
	AcceptDialog::set_swap_cancel_ok(swap_cancel_ok);
#endif

	/* REGISTER ANIMATION */
	GDREGISTER_CLASS(Tween);
	GDREGISTER_ABSTRACT_CLASS(Tweener);
	GDREGISTER_CLASS(PropertyTweener);
	GDREGISTER_CLASS(IntervalTweener);
	GDREGISTER_CLASS(CallbackTweener);
	GDREGISTER_CLASS(MethodTweener);

	GDREGISTER_ABSTRACT_CLASS(AnimationMixer);
	GDREGISTER_CLASS(AnimationPlayer);
	GDREGISTER_CLASS(AnimationTree);
	GDREGISTER_CLASS(AnimationNode);
	GDREGISTER_CLASS(AnimationRootNode);
	GDREGISTER_CLASS(AnimationNodeBlendTree);
	GDREGISTER_CLASS(AnimationNodeBlendSpace1D);
	GDREGISTER_CLASS(AnimationNodeBlendSpace2D);
	GDREGISTER_CLASS(AnimationNodeStateMachine);
	GDREGISTER_CLASS(AnimationNodeStateMachinePlayback);

	GDREGISTER_CLASS(AnimationNodeSync);
	GDREGISTER_CLASS(AnimationNodeStateMachineTransition);
	GDREGISTER_CLASS(AnimationNodeOutput);
	GDREGISTER_CLASS(AnimationNodeOneShot);
	GDREGISTER_CLASS(AnimationNodeAnimation);
	GDREGISTER_CLASS(AnimationNodeAdd2);
	GDREGISTER_CLASS(AnimationNodeAdd3);
	GDREGISTER_CLASS(AnimationNodeBlend2);
	GDREGISTER_CLASS(AnimationNodeBlend3);
	GDREGISTER_CLASS(AnimationNodeSub2);
	GDREGISTER_CLASS(AnimationNodeTimeScale);
	GDREGISTER_CLASS(AnimationNodeTimeSeek);
	GDREGISTER_CLASS(AnimationNodeTransition);

	GDREGISTER_CLASS(ShaderGlobalsOverride); // can be used in any shader

	OS::get_singleton()->yield(); // may take time to init

	/* REGISTER SHADER */

	GDREGISTER_CLASS(Shader);
	GDREGISTER_CLASS(VisualShader);
	GDREGISTER_CLASS(ShaderInclude);
	GDREGISTER_ABSTRACT_CLASS(VisualShaderNode);
	GDREGISTER_CLASS(VisualShaderNodeCustom);
	GDREGISTER_CLASS(VisualShaderNodeInput);
	GDREGISTER_ABSTRACT_CLASS(VisualShaderNodeOutput);
	GDREGISTER_ABSTRACT_CLASS(VisualShaderNodeResizableBase);
	GDREGISTER_ABSTRACT_CLASS(VisualShaderNodeGroupBase);
	GDREGISTER_ABSTRACT_CLASS(VisualShaderNodeConstant);
	GDREGISTER_ABSTRACT_CLASS(VisualShaderNodeVectorBase);
	GDREGISTER_CLASS(VisualShaderNodeComment);
	GDREGISTER_CLASS(VisualShaderNodeFloatConstant);
	GDREGISTER_CLASS(VisualShaderNodeIntConstant);
	GDREGISTER_CLASS(VisualShaderNodeUIntConstant);
	GDREGISTER_CLASS(VisualShaderNodeBooleanConstant);
	GDREGISTER_CLASS(VisualShaderNodeColorConstant);
	GDREGISTER_CLASS(VisualShaderNodeVec2Constant);
	GDREGISTER_CLASS(VisualShaderNodeVec3Constant);
	GDREGISTER_CLASS(VisualShaderNodeVec4Constant);
	GDREGISTER_CLASS(VisualShaderNodeTransformConstant);
	GDREGISTER_CLASS(VisualShaderNodeFloatOp);
	GDREGISTER_CLASS(VisualShaderNodeIntOp);
	GDREGISTER_CLASS(VisualShaderNodeUIntOp);
	GDREGISTER_CLASS(VisualShaderNodeVectorOp);
	GDREGISTER_CLASS(VisualShaderNodeColorOp);
	GDREGISTER_CLASS(VisualShaderNodeTransformOp);
	GDREGISTER_CLASS(VisualShaderNodeTransformVecMult);
	GDREGISTER_CLASS(VisualShaderNodeFloatFunc);
	GDREGISTER_CLASS(VisualShaderNodeIntFunc);
	GDREGISTER_CLASS(VisualShaderNodeUIntFunc);
	GDREGISTER_CLASS(VisualShaderNodeVectorFunc);
	GDREGISTER_CLASS(VisualShaderNodeColorFunc);
	GDREGISTER_CLASS(VisualShaderNodeTransformFunc);
	GDREGISTER_CLASS(VisualShaderNodeUVFunc);
	GDREGISTER_CLASS(VisualShaderNodeUVPolarCoord);
	GDREGISTER_CLASS(VisualShaderNodeDotProduct);
	GDREGISTER_CLASS(VisualShaderNodeVectorLen);
	GDREGISTER_CLASS(VisualShaderNodeDeterminant);
	GDREGISTER_CLASS(VisualShaderNodeDerivativeFunc);
	GDREGISTER_CLASS(VisualShaderNodeClamp);
	GDREGISTER_CLASS(VisualShaderNodeFaceForward);
	GDREGISTER_CLASS(VisualShaderNodeOuterProduct);
	GDREGISTER_CLASS(VisualShaderNodeSmoothStep);
	GDREGISTER_CLASS(VisualShaderNodeStep);
	GDREGISTER_CLASS(VisualShaderNodeVectorDistance);
	GDREGISTER_CLASS(VisualShaderNodeVectorRefract);
	GDREGISTER_CLASS(VisualShaderNodeMix);
	GDREGISTER_CLASS(VisualShaderNodeVectorCompose);
	GDREGISTER_CLASS(VisualShaderNodeTransformCompose);
	GDREGISTER_CLASS(VisualShaderNodeVectorDecompose);
	GDREGISTER_CLASS(VisualShaderNodeTransformDecompose);
	GDREGISTER_CLASS(VisualShaderNodeTexture);
	GDREGISTER_CLASS(VisualShaderNodeCurveTexture);
	GDREGISTER_CLASS(VisualShaderNodeCurveXYZTexture);
	GDREGISTER_ABSTRACT_CLASS(VisualShaderNodeSample3D);
	GDREGISTER_CLASS(VisualShaderNodeTexture2DArray);
	GDREGISTER_CLASS(VisualShaderNodeTexture3D);
	GDREGISTER_CLASS(VisualShaderNodeCubemap);
	GDREGISTER_ABSTRACT_CLASS(VisualShaderNodeParameter);
	GDREGISTER_CLASS(VisualShaderNodeParameterRef);
	GDREGISTER_CLASS(VisualShaderNodeFloatParameter);
	GDREGISTER_CLASS(VisualShaderNodeIntParameter);
	GDREGISTER_CLASS(VisualShaderNodeUIntParameter);
	GDREGISTER_CLASS(VisualShaderNodeBooleanParameter);
	GDREGISTER_CLASS(VisualShaderNodeColorParameter);
	GDREGISTER_CLASS(VisualShaderNodeVec2Parameter);
	GDREGISTER_CLASS(VisualShaderNodeVec3Parameter);
	GDREGISTER_CLASS(VisualShaderNodeVec4Parameter);
	GDREGISTER_CLASS(VisualShaderNodeTransformParameter);
	GDREGISTER_ABSTRACT_CLASS(VisualShaderNodeTextureParameter);
	GDREGISTER_CLASS(VisualShaderNodeTexture2DParameter);
	GDREGISTER_CLASS(VisualShaderNodeTextureParameterTriplanar);
	GDREGISTER_CLASS(VisualShaderNodeTexture2DArrayParameter);
	GDREGISTER_CLASS(VisualShaderNodeTexture3DParameter);
	GDREGISTER_CLASS(VisualShaderNodeCubemapParameter);
	GDREGISTER_CLASS(VisualShaderNodeLinearSceneDepth);
	GDREGISTER_CLASS(VisualShaderNodeWorldPositionFromDepth);
	GDREGISTER_CLASS(VisualShaderNodeScreenNormalWorldSpace);
	GDREGISTER_CLASS(VisualShaderNodeIf);
	GDREGISTER_CLASS(VisualShaderNodeSwitch);
	GDREGISTER_CLASS(VisualShaderNodeFresnel);
	GDREGISTER_CLASS(VisualShaderNodeExpression);
	GDREGISTER_CLASS(VisualShaderNodeGlobalExpression);
	GDREGISTER_CLASS(VisualShaderNodeIs);
	GDREGISTER_CLASS(VisualShaderNodeCompare);
	GDREGISTER_CLASS(VisualShaderNodeMultiplyAdd);
	GDREGISTER_CLASS(VisualShaderNodeBillboard);
	GDREGISTER_CLASS(VisualShaderNodeDistanceFade);
	GDREGISTER_CLASS(VisualShaderNodeProximityFade);
	GDREGISTER_CLASS(VisualShaderNodeRandomRange);
	GDREGISTER_CLASS(VisualShaderNodeRemap);
	GDREGISTER_CLASS(VisualShaderNodeRotationByAxis);
	GDREGISTER_ABSTRACT_CLASS(VisualShaderNodeVarying);
	GDREGISTER_CLASS(VisualShaderNodeVaryingSetter);
	GDREGISTER_CLASS(VisualShaderNodeVaryingGetter);

	GDREGISTER_CLASS(VisualShaderNodeSDFToScreenUV);
	GDREGISTER_CLASS(VisualShaderNodeScreenUVToSDF);
	GDREGISTER_CLASS(VisualShaderNodeTextureSDF);
	GDREGISTER_CLASS(VisualShaderNodeTextureSDFNormal);
	GDREGISTER_CLASS(VisualShaderNodeSDFRaymarch);

	GDREGISTER_CLASS(VisualShaderNodeParticleOutput);
	GDREGISTER_ABSTRACT_CLASS(VisualShaderNodeParticleEmitter);
	GDREGISTER_CLASS(VisualShaderNodeParticleSphereEmitter);
	GDREGISTER_CLASS(VisualShaderNodeParticleBoxEmitter);
	GDREGISTER_CLASS(VisualShaderNodeParticleRingEmitter);
	GDREGISTER_CLASS(VisualShaderNodeParticleMeshEmitter);
	GDREGISTER_CLASS(VisualShaderNodeParticleMultiplyByAxisAngle);
	GDREGISTER_CLASS(VisualShaderNodeParticleConeVelocity);
	GDREGISTER_CLASS(VisualShaderNodeParticleRandomness);
	GDREGISTER_CLASS(VisualShaderNodeParticleAccelerator);
	GDREGISTER_CLASS(VisualShaderNodeParticleEmit);

	GDREGISTER_VIRTUAL_CLASS(Material);
	GDREGISTER_CLASS(PlaceholderMaterial);
	GDREGISTER_CLASS(ShaderMaterial);
	GDREGISTER_ABSTRACT_CLASS(CanvasItem);
	GDREGISTER_CLASS(CanvasTexture);
	GDREGISTER_CLASS(CanvasItemMaterial);
	SceneTree::add_idle_callback(CanvasItemMaterial::flush_changes);
	CanvasItemMaterial::init_shaders();

	/* REGISTER 2D */

	GDREGISTER_CLASS(Node2D);
	GDREGISTER_CLASS(CanvasGroup);
	GDREGISTER_CLASS(CPUParticles2D);
	GDREGISTER_CLASS(GPUParticles2D);
	GDREGISTER_CLASS(Sprite2D);
	GDREGISTER_CLASS(SpriteFrames);
	GDREGISTER_CLASS(AnimatedSprite2D);
	GDREGISTER_CLASS(Marker2D);
	GDREGISTER_CLASS(Line2D);
	GDREGISTER_ABSTRACT_CLASS(CollisionObject2D);
	GDREGISTER_ABSTRACT_CLASS(PhysicsBody2D);
	GDREGISTER_CLASS(StaticBody2D);
	GDREGISTER_CLASS(AnimatableBody2D);
	GDREGISTER_CLASS(RigidBody2D);
	GDREGISTER_CLASS(CharacterBody2D);
	GDREGISTER_CLASS(KinematicCollision2D);
	GDREGISTER_CLASS(Area2D);
	GDREGISTER_CLASS(CollisionShape2D);
	GDREGISTER_CLASS(CollisionPolygon2D);
	GDREGISTER_CLASS(RayCast2D);
	GDREGISTER_CLASS(ShapeCast2D);
	GDREGISTER_CLASS(VisibleOnScreenNotifier2D);
	GDREGISTER_CLASS(VisibleOnScreenEnabler2D);
	GDREGISTER_CLASS(Polygon2D);
	GDREGISTER_CLASS(Skeleton2D);
	GDREGISTER_CLASS(Bone2D);
	GDREGISTER_ABSTRACT_CLASS(Light2D);
	GDREGISTER_CLASS(PointLight2D);
	GDREGISTER_CLASS(DirectionalLight2D);
	GDREGISTER_CLASS(LightOccluder2D);
	GDREGISTER_CLASS(OccluderPolygon2D);
	GDREGISTER_CLASS(BackBufferCopy);

	OS::get_singleton()->yield(); // may take time to init

	GDREGISTER_CLASS(Camera2D);
	GDREGISTER_CLASS(AudioListener2D);
	GDREGISTER_ABSTRACT_CLASS(Joint2D);
	GDREGISTER_CLASS(PinJoint2D);
	GDREGISTER_CLASS(GrooveJoint2D);
	GDREGISTER_CLASS(DampedSpringJoint2D);
	GDREGISTER_CLASS(TileSet);
	GDREGISTER_ABSTRACT_CLASS(TileSetSource);
	GDREGISTER_CLASS(TileSetAtlasSource);
	GDREGISTER_CLASS(TileSetScenesCollectionSource);
	GDREGISTER_CLASS(TileMapPattern);
	GDREGISTER_CLASS(TileData);
	GDREGISTER_CLASS(TileMap);
	GDREGISTER_CLASS(ParallaxBackground);
	GDREGISTER_CLASS(ParallaxLayer);
	GDREGISTER_CLASS(TouchScreenButton);
	GDREGISTER_CLASS(RemoteTransform2D);

	GDREGISTER_CLASS(SkeletonModificationStack2D);
	GDREGISTER_CLASS(SkeletonModification2D);
	GDREGISTER_CLASS(SkeletonModification2DLookAt);
	GDREGISTER_CLASS(SkeletonModification2DCCDIK);
	GDREGISTER_CLASS(SkeletonModification2DFABRIK);
	GDREGISTER_CLASS(SkeletonModification2DJiggle);
	GDREGISTER_CLASS(SkeletonModification2DTwoBoneIK);
	GDREGISTER_CLASS(SkeletonModification2DStackHolder);

	GDREGISTER_CLASS(PhysicalBone2D);
	GDREGISTER_CLASS(SkeletonModification2DPhysicalBones);

	OS::get_singleton()->yield(); // may take time to init

	/* REGISTER RESOURCES */

	GDREGISTER_ABSTRACT_CLASS(Shader);
	GDREGISTER_CLASS(ParticleProcessMaterial);
	SceneTree::add_idle_callback(ParticleProcessMaterial::flush_changes);
	ParticleProcessMaterial::init_shaders();

	GDREGISTER_VIRTUAL_CLASS(Mesh);
	GDREGISTER_CLASS(MeshConvexDecompositionSettings);
	GDREGISTER_CLASS(ArrayMesh);
	GDREGISTER_CLASS(PlaceholderMesh);
	GDREGISTER_CLASS(MultiMesh);
	GDREGISTER_CLASS(MeshDataTool);

	GDREGISTER_CLASS(PhysicsMaterial);
	GDREGISTER_CLASS(World2D);
	GDREGISTER_VIRTUAL_CLASS(Texture);
	GDREGISTER_VIRTUAL_CLASS(Texture2D);
	GDREGISTER_CLASS(CompressedTexture2D);
	GDREGISTER_CLASS(PortableCompressedTexture2D);
	GDREGISTER_CLASS(ImageTexture);
	GDREGISTER_CLASS(AtlasTexture);
	GDREGISTER_CLASS(CurveTexture);
	GDREGISTER_CLASS(CurveXYZTexture);
	GDREGISTER_CLASS(GradientTexture1D);
	GDREGISTER_CLASS(GradientTexture2D);
	GDREGISTER_CLASS(AnimatedTexture);
	GDREGISTER_CLASS(CameraTexture);
	GDREGISTER_VIRTUAL_CLASS(TextureLayered);
	GDREGISTER_ABSTRACT_CLASS(ImageTextureLayered);
	GDREGISTER_VIRTUAL_CLASS(Texture3D);
	GDREGISTER_CLASS(ImageTexture3D);
	GDREGISTER_CLASS(CompressedTexture3D);
	GDREGISTER_CLASS(Cubemap);
	GDREGISTER_CLASS(CubemapArray);
	GDREGISTER_CLASS(Texture2DArray);
	GDREGISTER_ABSTRACT_CLASS(CompressedTextureLayered);
	GDREGISTER_CLASS(CompressedCubemap);
	GDREGISTER_CLASS(CompressedCubemapArray);
	GDREGISTER_CLASS(CompressedTexture2DArray);
	GDREGISTER_CLASS(PlaceholderTexture2D);
	GDREGISTER_CLASS(PlaceholderTexture3D);
	GDREGISTER_ABSTRACT_CLASS(PlaceholderTextureLayered);
	GDREGISTER_CLASS(PlaceholderTexture2DArray);
	GDREGISTER_CLASS(PlaceholderCubemap);
	GDREGISTER_CLASS(PlaceholderCubemapArray);

	// These classes are part of renderer_rd
	GDREGISTER_CLASS(Texture2DRD);
	GDREGISTER_ABSTRACT_CLASS(TextureLayeredRD);
	GDREGISTER_CLASS(Texture2DArrayRD);
	GDREGISTER_CLASS(TextureCubemapRD);
	GDREGISTER_CLASS(TextureCubemapArrayRD);
	GDREGISTER_CLASS(Texture3DRD);

	GDREGISTER_CLASS(Animation);
	GDREGISTER_CLASS(AnimationLibrary);

	GDREGISTER_ABSTRACT_CLASS(Font);
	GDREGISTER_CLASS(FontFile);
	GDREGISTER_CLASS(FontVariation);
	GDREGISTER_CLASS(SystemFont);

	GDREGISTER_CLASS(Curve);

	GDREGISTER_CLASS(LabelSettings);

	GDREGISTER_CLASS(TextLine);
	GDREGISTER_CLASS(TextParagraph);

	GDREGISTER_VIRTUAL_CLASS(StyleBox);
	GDREGISTER_CLASS(StyleBoxEmpty);
	GDREGISTER_CLASS(StyleBoxTexture);
	GDREGISTER_CLASS(StyleBoxFlat);
	GDREGISTER_CLASS(StyleBoxLine);
	GDREGISTER_CLASS(Theme);

	GDREGISTER_CLASS(PolygonPathFinder);
	GDREGISTER_CLASS(BitMap);
	GDREGISTER_CLASS(Gradient);

	OS::get_singleton()->yield(); // may take time to init

	GDREGISTER_CLASS(AudioStreamPlayer);
	GDREGISTER_CLASS(AudioStreamPlayer2D);
	GDREGISTER_CLASS(AudioStreamWAV);
	GDREGISTER_CLASS(AudioStreamPolyphonic);
	GDREGISTER_ABSTRACT_CLASS(AudioStreamPlaybackPolyphonic);

	OS::get_singleton()->yield(); // may take time to init

	GDREGISTER_ABSTRACT_CLASS(Shape2D);
	GDREGISTER_CLASS(WorldBoundaryShape2D);
	GDREGISTER_CLASS(SegmentShape2D);
	GDREGISTER_CLASS(SeparationRayShape2D);
	GDREGISTER_CLASS(CircleShape2D);
	GDREGISTER_CLASS(RectangleShape2D);
	GDREGISTER_CLASS(CapsuleShape2D);
	GDREGISTER_CLASS(ConvexPolygonShape2D);
	GDREGISTER_CLASS(ConcavePolygonShape2D);
	GDREGISTER_CLASS(Curve2D);
	GDREGISTER_CLASS(Path2D);
	GDREGISTER_CLASS(PathFollow2D);

	OS::get_singleton()->yield(); // may take time to init

	GDREGISTER_ABSTRACT_CLASS(SceneState);
	GDREGISTER_CLASS(PackedScene);

	GDREGISTER_CLASS(SceneTree);
	GDREGISTER_ABSTRACT_CLASS(SceneTreeTimer); // sorry, you can't create it

#ifndef DISABLE_DEPRECATED
	// Dropped in 4.0, near approximation.
	ClassDB::add_compatibility_class("AnimationTreePlayer", "AnimationTree");
	ClassDB::add_compatibility_class("BitmapFont", "FontFile");
	ClassDB::add_compatibility_class("DynamicFont", "FontFile");
	ClassDB::add_compatibility_class("DynamicFontData", "FontFile");
	ClassDB::add_compatibility_class("OpenSimplexNoise", "FastNoiseLite");
	ClassDB::add_compatibility_class("ToolButton", "Button");
	ClassDB::add_compatibility_class("YSort", "Node2D");
	// The OccluderShapeSphere resource (used in the old Occluder node) is not present anymore.
	ClassDB::add_compatibility_class("OccluderShapeSphere", "Resource");

	// Renamed in 4.0.
	// Keep alphabetical ordering to easily locate classes and avoid duplicates.
	ClassDB::add_compatibility_class("AnimatedSprite", "AnimatedSprite2D");
	ClassDB::add_compatibility_class("CubeMesh", "BoxMesh");
	ClassDB::add_compatibility_class("GradientTexture", "GradientTexture1D");
	// ClassDB::add_compatibility_class("HingeJoint", "HingeJoint3D");
	ClassDB::add_compatibility_class("Joint", "Joint3D");
	// ClassDB::add_compatibility_class("KinematicBody", "CharacterBody3D");
	ClassDB::add_compatibility_class("KinematicBody2D", "CharacterBody2D");
	// ClassDB::add_compatibility_class("KinematicCollision", "KinematicCollision3D");
	ClassDB::add_compatibility_class("Light2D", "PointLight2D");
	ClassDB::add_compatibility_class("LineShape2D", "WorldBoundaryShape2D");
	// ClassDB::add_compatibility_class("MeshInstance", "MeshInstance3D");
	// ClassDB::add_compatibility_class("MultiMeshInstance", "MultiMeshInstance3D");
	// ClassDB::add_compatibility_class("Particles", "GPUParticles3D");
	ClassDB::add_compatibility_class("Particles2D", "GPUParticles2D");
	ClassDB::add_compatibility_class("ParticlesMaterial", "ParticleProcessMaterial");
	// ClassDB::add_compatibility_class("PathFollow", "PathFollow3D");
	// ClassDB::add_compatibility_class("PhysicalBone", "PhysicalBone3D");
	ClassDB::add_compatibility_class("Physics2DDirectBodyState", "PhysicsDirectBodyState2D");
	ClassDB::add_compatibility_class("Physics2DDirectSpaceState", "PhysicsDirectSpaceState2D");
	ClassDB::add_compatibility_class("Physics2DServer", "PhysicsServer2D");
	ClassDB::add_compatibility_class("Physics2DShapeQueryParameters", "PhysicsShapeQueryParameters2D");
	ClassDB::add_compatibility_class("Physics2DTestMotionResult", "PhysicsTestMotionResult2D");
	// ClassDB::add_compatibility_class("PhysicsBody", "PhysicsBody3D");
	// ClassDB::add_compatibility_class("PhysicsDirectBodyState", "PhysicsDirectBodyState3D");
	// ClassDB::add_compatibility_class("PhysicsDirectSpaceState", "PhysicsDirectSpaceState3D");
	// ClassDB::add_compatibility_class("PhysicsServer", "PhysicsServer3D");
	// ClassDB::add_compatibility_class("PhysicsShapeQueryParameters", "PhysicsShapeQueryParameters3D");
	// ClassDB::add_compatibility_class("PinJoint", "PinJoint3D");
	ClassDB::add_compatibility_class("Position2D", "Marker2D");
	// ClassDB::add_compatibility_class("Position3D", "Marker3D");
	// ClassDB::add_compatibility_class("RayCast", "RayCast3D");
	ClassDB::add_compatibility_class("RayShape2D", "SeparationRayShape2D");
	// ClassDB::add_compatibility_class("RemoteTransform", "RemoteTransform3D");
	// ClassDB::add_compatibility_class("RigidBody", "RigidBody3D");
	ClassDB::add_compatibility_class("RigidDynamicBody2D", "RigidBody2D");
	// ClassDB::add_compatibility_class("RigidDynamicBody3D", "RigidBody3D");
	ClassDB::add_compatibility_class("ShortCut", "Shortcut");
	// ClassDB::add_compatibility_class("SpotLight", "SpotLight3D");
	// ClassDB::add_compatibility_class("SpringArm", "SpringArm3D");
	ClassDB::add_compatibility_class("Sprite", "Sprite2D");
	ClassDB::add_compatibility_class("StreamTexture", "CompressedTexture2D");
	ClassDB::add_compatibility_class("TextureProgress", "TextureProgressBar");
	// ClassDB::add_compatibility_class("VehicleBody", "VehicleBody3D");
	// ClassDB::add_compatibility_class("VehicleWheel", "VehicleWheel3D");
	ClassDB::add_compatibility_class("VideoPlayer", "VideoStreamPlayer");
	ClassDB::add_compatibility_class("ViewportContainer", "SubViewportContainer");
	ClassDB::add_compatibility_class("Viewport", "SubViewport");
	// ClassDB::add_compatibility_class("VisibilityEnabler", "VisibleOnScreenEnabler3D");
	// ClassDB::add_compatibility_class("VisibilityNotifier", "VisibleOnScreenNotifier3D");
	ClassDB::add_compatibility_class("VisibilityNotifier2D", "VisibleOnScreenNotifier2D");
	// ClassDB::add_compatibility_class("VisibilityNotifier3D", "VisibleOnScreenNotifier3D");
	ClassDB::add_compatibility_class("VisualServer", "RenderingServer");

	// VisualShader classes.
	ClassDB::add_compatibility_class("VisualShaderNodeScalarConstant", "VisualShaderNodeFloatConstant");
	ClassDB::add_compatibility_class("VisualShaderNodeScalarFunc", "VisualShaderNodeFloatFunc");
	ClassDB::add_compatibility_class("VisualShaderNodeScalarOp", "VisualShaderNodeFloatOp");
	ClassDB::add_compatibility_class("VisualShaderNodeScalarClamp", "VisualShaderNodeClamp");
	ClassDB::add_compatibility_class("VisualShaderNodeVectorClamp", "VisualShaderNodeClamp");
	ClassDB::add_compatibility_class("VisualShaderNodeScalarInterp", "VisualShaderNodeMix");
	ClassDB::add_compatibility_class("VisualShaderNodeVectorInterp", "VisualShaderNodeMix");
	ClassDB::add_compatibility_class("VisualShaderNodeVectorScalarMix", "VisualShaderNodeMix");
	ClassDB::add_compatibility_class("VisualShaderNodeScalarSmoothStep", "VisualShaderNodeSmoothStep");
	ClassDB::add_compatibility_class("VisualShaderNodeVectorSmoothStep", "VisualShaderNodeSmoothStep");
	ClassDB::add_compatibility_class("VisualShaderNodeVectorScalarSmoothStep", "VisualShaderNodeSmoothStep");
	ClassDB::add_compatibility_class("VisualShaderNodeVectorScalarStep", "VisualShaderNodeStep");
	ClassDB::add_compatibility_class("VisualShaderNodeScalarSwitch", "VisualShaderNodeSwitch");
	ClassDB::add_compatibility_class("VisualShaderNodeScalarTransformMult", "VisualShaderNodeTransformOp");
	ClassDB::add_compatibility_class("VisualShaderNodeScalarDerivativeFunc", "VisualShaderNodeDerivativeFunc");
	ClassDB::add_compatibility_class("VisualShaderNodeVectorDerivativeFunc", "VisualShaderNodeDerivativeFunc");

	ClassDB::add_compatibility_class("VisualShaderNodeBooleanUniform", "VisualShaderNodeBooleanParameter");
	ClassDB::add_compatibility_class("VisualShaderNodeColorUniform", "VisualShaderNodeColorParameter");
	ClassDB::add_compatibility_class("VisualShaderNodeScalarUniform", "VisualShaderNodeFloatParameter");
	ClassDB::add_compatibility_class("VisualShaderNodeCubeMapUniform", "VisualShaderNodeCubeMapParameter");
	ClassDB::add_compatibility_class("VisualShaderNodeTextureUniform", "VisualShaderNodeTexture2DParameter");
	ClassDB::add_compatibility_class("VisualShaderNodeTextureUniformTriplanar", "VisualShaderNodeTextureParameterTriplanar");
	ClassDB::add_compatibility_class("VisualShaderNodeTransformUniform", "VisualShaderNodeTransformParameter");
	ClassDB::add_compatibility_class("VisualShaderNodeVec3Uniform", "VisualShaderNodeVec3Parameter");
	ClassDB::add_compatibility_class("VisualShaderNodeUniform", "VisualShaderNodeParameter");
	ClassDB::add_compatibility_class("VisualShaderNodeUniformRef", "VisualShaderNodeParameterRef");

	// Renamed during 4.0 alpha, added to ease transition between alphas.
	ClassDB::add_compatibility_class("AudioStreamOGGVorbis", "AudioStreamOggVorbis");
	ClassDB::add_compatibility_class("AudioStreamSample", "AudioStreamWAV");
	ClassDB::add_compatibility_class("OGGPacketSequence", "OggPacketSequence");
	ClassDB::add_compatibility_class("StreamCubemap", "CompressedCubemap");
	ClassDB::add_compatibility_class("StreamCubemapArray", "CompressedCubemapArray");
	ClassDB::add_compatibility_class("StreamTexture2D", "CompressedTexture2D");
	ClassDB::add_compatibility_class("StreamTexture2DArray", "CompressedTexture2DArray");
	ClassDB::add_compatibility_class("StreamTexture3D", "CompressedTexture3D");
	ClassDB::add_compatibility_class("StreamTextureLayered", "CompressedTextureLayered");
	ClassDB::add_compatibility_class("VisualShaderNodeFloatUniform", "VisualShaderNodeFloatParameter");
#endif /* DISABLE_DEPRECATED */

	OS::get_singleton()->yield(); // may take time to init

	for (int i = 0; i < 20; i++) {
		GLOBAL_DEF_BASIC(vformat("%s/layer_%d", PNAME("layer_names/2d_render"), i + 1), "");
	}

	for (int i = 0; i < 32; i++) {
		GLOBAL_DEF_BASIC(vformat("%s/layer_%d", PNAME("layer_names/2d_physics"), i + 1), "");
	}

	if (RenderingServer::get_singleton()) {
		ColorPicker::init_shaders(); // RenderingServer needs to exist for this to succeed.
	}

	SceneDebugger::initialize();
}

void unregister_scene_types() {
	SceneDebugger::deinitialize();

	ResourceLoader::remove_resource_format_loader(resource_loader_texture_layered);
	resource_loader_texture_layered.unref();

	ResourceLoader::remove_resource_format_loader(resource_loader_texture_3d);
	resource_loader_texture_3d.unref();

	ResourceLoader::remove_resource_format_loader(resource_loader_stream_texture);
	resource_loader_stream_texture.unref();

	ResourceSaver::remove_resource_format_saver(resource_saver_text);
	resource_saver_text.unref();

	ResourceLoader::remove_resource_format_loader(resource_loader_text);
	resource_loader_text.unref();

	ResourceSaver::remove_resource_format_saver(resource_saver_shader);
	resource_saver_shader.unref();

	ResourceLoader::remove_resource_format_loader(resource_loader_shader);
	resource_loader_shader.unref();

	ResourceSaver::remove_resource_format_saver(resource_saver_shader_include);
	resource_saver_shader_include.unref();

	ResourceLoader::remove_resource_format_loader(resource_loader_shader_include);
	resource_loader_shader_include.unref();

	ParticleProcessMaterial::finish_shaders();
	CanvasItemMaterial::finish_shaders();
	ColorPicker::finish_shaders();
	SceneStringNames::free();
}

void register_scene_singletons() {
	GDREGISTER_CLASS(ThemeDB);

	Engine::get_singleton()->add_singleton(Engine::Singleton("ThemeDB", ThemeDB::get_singleton()));
}
