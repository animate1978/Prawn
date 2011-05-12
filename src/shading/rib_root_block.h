
/*
    Copyright 2008-2010, Romain Behar <romainbehar@users.sourceforge.net>

    This file is part of Shrimp 2.

    Shrimp 2 is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Shrimp 2 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Shrimp 2.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef _rib_root_block_h_
#define _rib_root_block_h_

#include "shader_block.h"
#include "scene.h"


class rib_root_block : public shader_block
{
public:
	rib_root_block (const std::string& Name, scene* Scene);

	std::string show_code();


	// show a preview of current scene
	void show_preview (const std::string& Directory);

	// export scene (RIB file and shaders)
	void export_scene (const std::string& Directory);

	// the type of root block (constant: RIB)
	const std::string root_type;

	// getters and setters
	void set_general_statements (const std::string& Statements);
	std::string get_general_statements();
	void set_imager_statement (const std::string& Statement);
	std::string get_imager_statement();

	void set_AOV (const bool State);
	bool get_AOV();

private:
	scene* m_scene;

	// RIB scene statements
	std::string m_general_statements;
	std::string m_imager_statement;

	// AOV output preview
	bool m_AOV;

	typedef enum
	{
		SURFACE,
		DISPLACEMENT,
		LIGHT,
		VOLUME,
		IMAGER,
	} shader_t;

	// write a RIB file for preview and image output
	void write_RIB (const std::string& RIBFile, const std::string& TempDir, const std::string& SurfaceName = "", const std::string& DisplacementName = "", const std::string& LightName = "", const std::string& AtmosphereName = "", const std::string& ImagerName = "");

	std::string shader_compilation_command (const std::string& Shader, const std::string& ShaderPath, const std::string& DestinationName, const std::string& DestinationPath, const std::string& IncludePath);
	std::string scene_rendering_command (const std::string& RIBFile, const std::string& ShaderPath);

	// parses a shader in the RIB scene and returns the shader compilation command
	std::string parse_scene_shader (const std::string& RIBscene, const std::string ShaderPath, const std::string& TempDir, const std::string ShaderType);

	// outputs scene and shader files, returns rendering command list
	typedef std::vector <std::string> command_list_t;
	void write_scene_and_shaders (const std::string& SceneDirectory, command_list_t& CommandList);

	// outputs rendering command list
	void write_command_list (const command_list_t& CommandList, const std::string& AbsoluteFileName);

	// export a shader to a RSL file
	bool export_shader (const shader_t ShaderType, const std::string& ShaderName, const std::string& ShaderFile);
	// export a K-3D slmeta file
	bool export_k3d_slmeta (const shader_t ShaderType, const std::string& ShaderName, const std::string& ShaderFile);


	// build a shader starting from he root block, returns the shader file content
	std::string build_shader_file (const shader_t ShaderType, const std::string& ShaderName);
	// recursively build the shader code ending at given block
	void build_shader_code (shader_block* Block, std::string& ShaderCode, std::set<std::string>& LocalVariables);
	// build the K-3D slmeta file for a shader
	std::string build_k3d_meta_file (const shader_t ShaderType, const std::string& ShaderName);

	// return the list of connected blocks that make the shader
	shrimp::shader_blocks_t get_all_shader_blocks (const shader_t ShaderType);

	// return true when the given pad input has an output connected
	bool has_connected_parent(const std::string& PadName);
};


#endif

