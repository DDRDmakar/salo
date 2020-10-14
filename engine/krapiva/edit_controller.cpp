/*
*
* SaloIntellect project
* Copyright (C) 2015-2020 Motylenok Mikhail, Makarevich Nikita
* 
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/

#include "headers/core.h"

using namespace Magick;

bool begins_with(const std::string &current_line, const std::string &target)
{
	return (int)current_line.find(target) == 0;
}

unsigned count_symbols(const std::string &plain_text)
{
	unsigned i = 0;
	for(auto &e : plain_text) if ((e & 0b11000000) != 0b10000000) ++i;
	return i;
}

std::string split_too_long_lines(std::string plain_text, const int max_line_length, const int max_lines_count)
{
	int last_free_space_index = 0;
	int last_break_index = 0;
	int lines_count = 1;
	
	for(int i = 0; i < plain_text.size(); ++i)
	{
		if(max_lines_count != 0 && lines_count > max_lines_count) throw std::invalid_argument("Error - too long text");
		
		if (count_symbols(plain_text.substr(last_break_index, i - last_break_index)) > max_line_length)
		{
			if (last_free_space_index != 0)
			{
				plain_text[last_free_space_index] = '\n';
				last_break_index = last_free_space_index;
				i = last_free_space_index;
				last_free_space_index = 0;
				++ lines_count;
				continue;
			}
			else if(plain_text[i] == ' ' || plain_text[i] == '\n')
			{
				last_break_index = i;
				last_free_space_index = 0;
				plain_text[i] = '\n';
				++ lines_count;
				continue;
			}
		}
		
		if (plain_text[i] == '\n')
		{
			last_break_index = i;
			last_free_space_index = 0;
			++ lines_count;
			continue;
		}
		
		if (plain_text[i] == ' ') last_free_space_index = i;
	}
	
	return plain_text;
}

void scale_to_cover_image(const Magick::Geometry &target, Magick::Image &current_image)
{
	const int pref_square_size = (target.width() > target.height()) ? target.width() : target.height();
	
	const float vh_coeff   = (float)target.width()  / target.height(); // Width/Height aspect ratio
	const float vh_coeff_1 = (float)current_image.size().width() / current_image.size().height();
	const bool vh_coeffs_are_similar = (vh_coeff < 1) ^ (vh_coeff_1 >= 1); // Width bigger then height or smaller then height on both images
	
	const float square_scale_coefficient = target.width() >= target.height() ?
		vh_coeffs_are_similar ? vh_coeff < vh_coeff_1 ? vh_coeff_1 / vh_coeff : 1 : 1 / vh_coeff_1 :
		vh_coeffs_are_similar ? vh_coeff > vh_coeff_1 ? vh_coeff / vh_coeff_1 : 1 : vh_coeff_1;
	
	current_image.scale(
		Magick::Geometry(
			pref_square_size * square_scale_coefficient,
			pref_square_size * square_scale_coefficient
		)
	);
}

void editor_controller(const std::string &path_to_binary, const std::string &path_to_task)
{
	YAML::Node task = YAML::Load(read_file(path_to_task + "task.txt"));
	
	// Check task format
	if (!task["filters"] || !task["filters"].IsSequence()) throw std::invalid_argument("Error - filters list not found");
	
	// Iterate through filters
	for (YAML::const_iterator it = task["filters"].begin(); it != task["filters"].end(); ++it)
	{
		const std::string filter_name = it->begin()->first.as<std::string>();
		const YAML::Node filter = it->begin()->second;
		
		std::cout<< "Processing filter \"" << filter_name << "\"\n";
		
// ### Flip
		if (filter_name == "flip")
		{
			/*
			 * _filename_in: ""
			 * _filename_out: ""
			 * ver: no/top/bottom
			 * hor: no/right/left
			 */
			
			if (
				filter["ver"].as<std::string>() != "top"  && filter["ver"].as<std::string>() != "bottom" &&
				filter["hor"].as<std::string>() != "left" && filter["hor"].as<std::string>() != "right"  &&
				filter["ver"].as<std::string>() != "no"   && filter["hor"].as<std::string>() != "no"
			) throw std::invalid_argument("Error - no flip options");
			
			// Read file from task, or by absolute path
			Magick::Image pic, pic2;
			pic.read((begins_with(filter["_filename_in"].as<std::string>(), "/") ? "" : path_to_task) + filter["_filename_in"].as<std::string>());
			const Magick::Geometry s = pic.size();
			
			if (filter["ver"])
			{
				pic2 = pic;
				pic2.flip();
				
				if (filter["ver"].as<std::string>() == "top")
				{
					pic2.chop(Magick::Geometry(0, s.height()/2 + 1));
					pic.composite(pic2, 0, s.height()/2, Magick::OverCompositeOp);
				}
				else if (filter["ver"].as<std::string>() == "bottom")
				{
					pic2.chop(Magick::Geometry(0, s.height()/2 + 1, 0, s.height()/2));
					pic.composite(pic2, 0, 0, Magick::OverCompositeOp);
				}
			}
			
			if (filter["hor"])
			{
				pic2 = pic;
				pic2.flop();
				
				if (filter["hor"].as<std::string>() == "right")
				{
					pic2.chop(Magick::Geometry(s.width()/2 + 1, 0, s.width()/2, 0));
					pic.composite(pic2, 0, 0, Magick::OverCompositeOp);
				}
				else if (filter["hor"].as<std::string>() == "left")
				{
					pic2.chop(Magick::Geometry(s.width()/2 + 1, 0));
					pic.composite(pic2, s.width()/2, 0, Magick::OverCompositeOp);
				}
			}
			
			pic.write((begins_with(filter["_filename_out"].as<std::string>(), "/") ? "" : path_to_task) + filter["_filename_out"].as<std::string>());
		}
		
// ### Annotate
		if (filter_name == "annotate")
		{
			/*
			 * _filename_out: ""
			 * text: ""
			 * max_line_length: 0
			 * max_lines_count: 0
			 * _width: 0
			 * _height: 0
			 * color_text: ""
			 *~color_stroke: ""
			 * color_background: ""
			 *~_font: ""
			 * font_size: 0
			 *~font_stroke_width: 0
			 */
			
			// Separate long lines
			const std::string plain_text = split_too_long_lines(
				filter["text"].as<std::string>(),
				filter["max_line_length"].as<int>(),
				filter["max_lines_count"].as<int>()
			);
			
			// New picture
			Magick::Image pic(
				Magick::Geometry(
					filter["_width"].as<int>(),
					filter["_height"].as<int>()
				),
				Magick::Color(filter["color_background"].as<std::string>())
			);
			
			// Set text color
			pic.fillColor(Magick::Color(filter["color_text"].as<std::string>()));
			if (filter["color_stroke"]) pic.strokeColor(Magick::Color(filter["color_stroke"].as<std::string>()));
			else pic.strokeColor(Magick::Color(filter["color_text"].as<std::string>()));
			
			// Set font
			if (filter["_font"]) pic.font((begins_with(filter["_font"].as<std::string>(), "/") ? "" : path_to_binary) + filter["_font"].as<std::string>());
			pic.fontPointsize(filter["font_size"].as<int>());
			if (filter["font_stroke_width"]) pic.strokeWidth(filter["font_stroke_width"].as<int>());
			else pic.strokeWidth(0);
			
			// Add text on image
			pic.annotate(
				plain_text,
				CenterGravity
			);
			
			// Write to file
			pic.write((begins_with(filter["_filename_out"].as<std::string>(), "/") ? "" : path_to_task) + filter["_filename_out"].as<std::string>());
		}
		
// ### Annotate photo
// Works with photo from resources
		if (filter_name == "annotate_image")
		{
			/*
			 * _filename_in:  ""
			 * _filename_out: ""
			 * text: ""
			 * max_line_length: 0
			 * max_lines_count: 0
			 * color_text: ""
			 *~color_stroke: ""
			 *~_font: ""
			 * font_size: 0
			 *~font_stroke_width: 0
			 */
			
			// Read background from resources or by absolute path
			Magick::Image background_image;
			background_image.read((begins_with(filter["_filename_in"].as<std::string>(), "/") ? "" : path_to_binary) + filter["_filename_in"].as<std::string>());
			const Magick::Geometry s = background_image.size();
			
			// Separate long lines
			const std::string plain_text = split_too_long_lines(
				filter["text"].as<std::string>(),
				filter["max_line_length"].as<int>(),
				filter["max_lines_count"].as<int>()
			);
			
			// New picture with text
			Magick::Image pic(
				Magick::Geometry(
					s.width(),
					s.height()
				),
				Magick::Color()
			);
			pic.texture(background_image);
			
			// Set text color
			pic.fillColor(Magick::Color(filter["color_text"].as<std::string>()));
			if (filter["color_stroke"]) pic.strokeColor(Magick::Color(filter["color_stroke"].as<std::string>()));
			else pic.strokeColor(Magick::Color(filter["color_text"].as<std::string>()));
			
			// Set font
			if (filter["_font"]) pic.font((begins_with(filter["_font"].as<std::string>(), "/") ? "" : path_to_binary) + filter["_font"].as<std::string>());
			pic.fontPointsize(filter["font_size"].as<int>());
			if (filter["font_stroke_width"]) pic.strokeWidth(filter["font_stroke_width"].as<int>());
			else pic.strokeWidth(0);
			
			// Add text on image
			pic.annotate(
				plain_text,
				CenterGravity
			);
			
			// Write to file
			pic.write((begins_with(filter["_filename_out"].as<std::string>(), "/") ? "" : path_to_task) + filter["_filename_out"].as<std::string>());
			
		}
		
// ### Draw text (auto scaleable text, but not centered)
		if (filter_name == "text_on_color")
		{
			/*
			 * _filename_out: ""
			 * text: ""
			 * width: 0
			 * height: 0
			 * border: 0.0
			 * color_text: ""
			 *~color_stroke: ""
			 * color_background: ""
			 *~_font: ""
			 *~font_stroke_width: 0
			 */
			
			// New picture
			Magick::Image pic(
				Magick::Geometry(
					filter["_width"].as<int>(),
					filter["_height"].as<int>()
				),
				Magick::Color()
			);
			
			// Set color
			pic.backgroundColor(Magick::Color(filter["color_background"].as<std::string>()));
			pic.borderColor(Magick::Color(filter["color_background"].as<std::string>()));
			
			// Set text color
			pic.fillColor(Magick::Color(filter["color_text"].as<std::string>()));
			if (filter["color_stroke"]) pic.strokeColor(Magick::Color(filter["color_stroke"].as<std::string>()));
			else pic.strokeColor(Magick::Color(filter["color_text"].as<std::string>()));
			
			// Set font
			if (filter["_font"]) pic.font((begins_with(filter["_font"].as<std::string>(), "/") ? "" : path_to_binary) + filter["_font"].as<std::string>());
			if (filter["font_stroke_width"]) pic.strokeWidth(filter["font_stroke_width"].as<int>());
			else pic.strokeWidth(0);
			
			// Add text
			pic.read("CAPTION:" + filter["text"].as<std::string>());
			pic.border(
				Magick::Geometry(
					filter["_width"].as<float>() * filter["border"].as<float>(),
					filter["_height"].as<float>() * filter["border"].as<float>()
				)
			);
			
			pic.write((begins_with(filter["_filename_out"].as<std::string>(), "/") ? "" : path_to_task) + filter["_filename_out"].as<std::string>());
		}
		
// ### Draw text on photo (auto scaleable text)
// Works with photo from task
// 		
// 		if (filter_name == "text_on_image")
// 		{
// 			/*
// 			 * _filename_in: ""
// 			 * _filename_out: ""
// 			 * text: ""
// 			 * border: 0.0
// 			 * color_text: ""
// 			 *~color_stroke: ""
// 			 *~_font: ""
// 			 *~font_stroke_width: 0
// 			 */
// 			
// 			// Read background from task, or by absolute path
// 			Magick::Image background_image;
// 			background_image.read((begins_with(filter["_filename_in"].as<std::string>(), "/") ? "" : path_to_task) + filter["_filename_in"].as<std::string>());
// 			const Magick::Geometry s = background_image.size();
// 			
// 			// New picture with text
// 			Magick::Image pic(
// 				Magick::Geometry(
// 					s.width(),
// 					s.height()
// 				),
// 				Magick::Color()
// 			);
// 			//pic.opacity(000);
// 			
// 			// Set text color
// 			pic.fillColor(Magick::Color(filter["color_text"].as<std::string>()));
// 			if (filter["color_stroke"]) pic.strokeColor(Magick::Color(filter["color_stroke"].as<std::string>()));
// 			else pic.strokeColor(Magick::Color(filter["color_text"].as<std::string>()));
// 			
// 			// Set font
// 			if (filter["_font"]) pic.font((begins_with(filter["_font"].as<std::string>(), "/") ? "" : path_to_binary) + filter["_font"].as<std::string>());
// 			if (filter["font_stroke_width"]) pic.strokeWidth(filter["font_stroke_width"].as<int>());
// 			else pic.strokeWidth(0);
// 			
// 			pic.texture(background_image);
// 			
// 			// Add text
// 			pic.read("CAPTION:" + filter["text"].as<std::string>());
// 			pic.border(
// 				Magick::Geometry(
// 					s.width() * filter["border"].as<float>(),
// 					s.height() * filter["border"].as<float>()
// 				)
// 			);
// 			
// 			
// 			pic.write((begins_with(filter["_filename_out"].as<std::string>(), "/") ? "" : path_to_task) + filter["_filename_out"].as<std::string>());
// 		}
// 		
		if (filter_name == "shakal")
		{
			/*
			* _filename_in: ""
			* _filename_out: ""
			* _size: 0
			* quality: 0
			*/
			
			// img2.compressType(MagickLib::JPEGCompression); img2.quality(20); //set JPEG compression at 20%
			
			// Read file from task, or by absolute path
			Magick::Image pic;
			pic.read((begins_with(filter["_filename_in"].as<std::string>(), "/") ? "" : path_to_task) + filter["_filename_in"].as<std::string>());
			const Magick::Geometry s = pic.size();
			
			// Resize image
			
			// Max width or height in pixels
			float new_pic_size = filter["_size"].as<float>();
			// resizing rectangle
			if (s.width() > new_pic_size || s.height() > new_pic_size)
			{
				pic.resize(
					Magick::Geometry(
						new_pic_size,
						new_pic_size
					)
				);
			}
			
			// Set quality level
			pic.quality(filter["quality"].as<float>());
			// Write out compressed JPEG
			pic.write((begins_with(filter["_filename_out"].as<std::string>(), "/") ? "" : path_to_task) + filter["_filename_out"].as<std::string>() + ".temp.jpg");
			// Read compressed JPEG
			pic.read((begins_with(filter["_filename_out"].as<std::string>(), "/") ? "" : path_to_task) + filter["_filename_out"].as<std::string>() + ".temp.jpg");
			// And write out result (could be not only jpg)
			pic.write((begins_with(filter["_filename_out"].as<std::string>(), "/") ? "" : path_to_task) + filter["_filename_out"].as<std::string>());
		}
		
		if (filter_name == "demotivator")
		{
			/* 
			 * _filename_in: ""
			 * _filename_out: ""
			 * text: ""
			 *~_font: ""
			 *~font_stroke_width: 0
			 *~color_text: ""
			 *~color_stroke: ""
			 *~color_background: ""
			 * border_size: 0.0
			 * text_area_height: 0.0
			 */
			
			// Read file from task, or by absolute path
			Magick::Image pic;
			pic.read((begins_with(filter["_filename_in"].as<std::string>(), "/") ? "" : path_to_task) + filter["_filename_in"].as<std::string>());
			const Magick::Geometry s = pic.size();
			
			// Create background
			Magick::Image background(
				Magick::Geometry(
					s.width() *  (1 + 2*filter["border_size"].as<float>()),
					s.height() * (1 + 2*filter["border_size"].as<float>() + filter["text_area_height"].as<float>())
				),
				Magick::Color(filter["color_background"] ? filter["color_background"].as<std::string>() : "black")
			);
			
			// Draw frame
			background.strokeColor(filter["color_text"] ? filter["color_text"].as<std::string>() : "white");
			background.strokeWidth(s.width() * filter["border_size"].as<float>()/12);
			background.draw(
				DrawableRectangle(
					s.width()  * filter["border_size"].as<float>()/3*2,
					s.height() * filter["border_size"].as<float>()/3*2,
					s.width()  * (1 + filter["border_size"].as<float>()*1.33),
					s.height() * (1 + filter["border_size"].as<float>()*1.33)
				)
			);
			
			// Create text on given color
			Magick::Image label(
				Magick::Geometry(
					s.width(),
					s.height() * filter["text_area_height"].as<float>()
				),
				Magick::Color()
			);
			
			// Set label color
			label.backgroundColor(Magick::Color(filter["color_background"] ? filter["color_background"].as<std::string>() : "black"));
			label.borderColor    (Magick::Color(filter["color_background"] ? filter["color_background"].as<std::string>() : "black"));
			
			// Set text color
			label.fillColor(Magick::Color(filter["color_text"] ? filter["color_text"].as<std::string>() : "white"));
			if (filter["color_stroke"]) label.strokeColor(Magick::Color(filter["color_stroke"].as<std::string>()));
			else label.strokeColor(Magick::Color(filter["color_text"] ? filter["color_text"].as<std::string>() : "white"));
			
			// Set font
			if (filter["_font"]) label.font((begins_with(filter["_font"].as<std::string>(), "/") ? "" : path_to_binary) + filter["_font"].as<std::string>());
			if (filter["font_stroke_width"]) label.strokeWidth(filter["font_stroke_width"].as<int>());
			else label.strokeWidth(0);
			
			// Add text
			label.read("CAPTION:" + filter["text"].as<std::string>());
			
			// Assemble image
			//     Composite image on background
			background.composite(
				pic,
				s.width()  * filter["border_size"].as<float>(),
				s.height() * filter["border_size"].as<float>(),
				Magick::OverCompositeOp
			);
			//     Composite text on background
			background.composite(
				label,
				s.width()  * filter["border_size"].as<float>(),
				s.height() * (1 + 2*filter["border_size"].as<float>()),
				Magick::OverCompositeOp
			);
			
			// Save result
			background.write((begins_with(filter["_filename_out"].as<std::string>(), "/") ? "" : path_to_task) + filter["_filename_out"].as<std::string>());
		}
		
		
		if (filter_name == "spasibo_konchil")
		{
			/*
			* _filename_in: ""
			* _filename_out: ""
			*/
			
			// Read template from resources
			Magick::Image templ;
			templ.read(path_to_binary + "resources/sperm.png");
			const Magick::Geometry s1 = templ.size();
			
			// Read file from task, or by absolute path
			Magick::Image pic;
			pic.read((begins_with(filter["_filename_in"].as<std::string>(), "/") ? "" : path_to_task) + filter["_filename_in"].as<std::string>());
			const Magick::Geometry s2 = pic.size();
			
			// Backgroung image
			Magick::Image background(
				Magick::Geometry(s1.width(), s1.height()),
				Magick::Color("white")
			);
			
			// Composite
			if (s2.width() > s2.height())
				pic.scale(Magick::Geometry(580, 580));
			else
				pic.scale(Magick::Geometry(
					580.0 * (float)s2.height() / (float)s2.width(),
					580.0 * (float)s2.height() / (float)s2.width()
				));
			
			background.composite(pic, 590, 300);
			background.composite(templ, 0, 0, Magick::OverCompositeOp);
			
			// Save result
			background.write((begins_with(filter["_filename_out"].as<std::string>(), "/") ? "" : path_to_task) + filter["_filename_out"].as<std::string>());
		}
		
		if (filter_name == "tnn")
		{
			/*
			* _filename_in: ""
			* _filename_out: ""
			*/
			
			// Read template from resources
			Magick::Image templ;
			templ.read(path_to_binary + "resources/tnn.png");
			const Magick::Geometry s1 = templ.size();
			
			// Read file from task, or by absolute path
			Magick::Image pic;
			pic.read((begins_with(filter["_filename_in"].as<std::string>(), "/") ? "" : path_to_task) + filter["_filename_in"].as<std::string>());
			const Magick::Geometry s2 = pic.size();
			
			// Backgroung image
			Magick::Image background(
				Magick::Geometry(s1.width(), s1.height()),
				Magick::Color("white")
			);
			
			// Composite
			if (s2.width() > s2.height())
				pic.scale(Magick::Geometry(170, 170));
			else
			{
				pic.scale(Magick::Geometry(
					170.0 * (float)s2.height() / (float)s2.width(),
					170.0 * (float)s2.height() / (float)s2.width()
				));
			}
			
			int ver_shift = (pic.size().height() - 100) / 2;
			background.composite(pic, 400, 100 - ver_shift);
			background.composite(templ, 0, 0, Magick::OverCompositeOp);
			
			// Save result
			background.write((begins_with(filter["_filename_out"].as<std::string>(), "/") ? "" : path_to_task) + filter["_filename_out"].as<std::string>());
		}
		
		if (filter_name == "meme")
		{
			/* 
			 * text_1: ""
			 * text_2: ""
			 * 
			 * _filename_in: ""
			 * _filename_out: ""
			 * max_line_length: 0
			 * max_lines_count: 0
			 * _width: 0
			 *~_font: ""
			 * font_size: 0
			 *~font_stroke_width: 0
			 * color_text: ""
			 *~color_stroke: ""
			 */
			
			// Separate long lines
			const std::string plain_text_1 = split_too_long_lines(
				filter["text_1"].as<std::string>(),
				filter["max_line_length"].as<int>(),
				filter["max_lines_count"].as<int>()
			);
			const std::string plain_text_2 = split_too_long_lines(
				filter["text_2"].as<std::string>(),
				filter["max_line_length"].as<int>(),
				filter["max_lines_count"].as<int>()
			);
			
			// Read file from task, or by absolute path
			Magick::Image pic;
			pic.read((begins_with(filter["_filename_in"].as<std::string>(), "/") ? "" : path_to_task) + filter["_filename_in"].as<std::string>());
			const Magick::Geometry s = pic.size();
			
			// New meme width
			const int pref_width = filter["_width"].as<int>();
			
			// Resize image to fit standard meme width
			// TODO
			if (s.width() > s.height())
				pic.scale(Magick::Geometry(pref_width, pref_width));
			else
				pic.scale(Magick::Geometry(
					(float)pref_width * (float)s.height() / (float)s.width(),
					(float)pref_width * (float)s.height() / (float)s.width()
				));
			
			const Magick::Geometry s1 = pic.size();
			
			// Set text color
			pic.fillColor(Magick::Color(filter["color_text"].as<std::string>()));
			if (filter["color_stroke"]) pic.strokeColor(Magick::Color(filter["color_stroke"].as<std::string>()));
			else pic.strokeColor(Magick::Color(filter["color_text"].as<std::string>()));
			
			// Set font
			if (filter["_font"]) pic.font((begins_with(filter["_font"].as<std::string>(), "/") ? "" : path_to_binary) + filter["_font"].as<std::string>());
			pic.fontPointsize(filter["font_size"].as<int>());
			if (filter["font_stroke_width"]) pic.strokeWidth(filter["font_stroke_width"].as<int>());
			
			// Write text on image
			pic.annotate(
				plain_text_1,
				NorthGravity
			);
			pic.annotate(
				plain_text_2,
				SouthGravity
			);
			
			// Save result
			pic.write((begins_with(filter["_filename_out"].as<std::string>(), "/") ? "" : path_to_task) + filter["_filename_out"].as<std::string>());
		}
		
		if (filter_name == "transparent_merge")
		{
			/* 
			 * _filename_in: ""
			 * _filename_out: ""
			 * _filename_theme: ""
			 * transparency: 0.0
			 * grayscale: no
			 */
			
			// Read file from task, or by absolute path
			Magick::Image pic;
			pic.read((begins_with(filter["_filename_in"].as<std::string>(), "/") ? "" : path_to_task) + filter["_filename_in"].as<std::string>());
			const Magick::Geometry s = pic.size();
			
			// Read theme picture from resources, or by absolute path
			Magick::Image pic_theme;
			pic_theme.read((begins_with(filter["_filename_theme"].as<std::string>(), "/") ? "" : path_to_binary) + filter["_filename_theme"].as<std::string>());
			const Magick::Geometry s1 = pic_theme.size();
			
			// Resize theme picture to fit given picture
			scale_to_cover_image(s, pic_theme);
			
			// Grayscale given image, if necessary
			if (filter["grayscale"].as<bool>()) pic.type(Magick::GrayscaleType);
			
			// Add transparency to theme picture
			pic_theme.opacity(TransparentOpacity * filter["transparency"].as<float>());
			
			// Merge pictures
			const int composite_shift_hor = (s.width()  - pic_theme.size().width())  / 2;
			const int composite_shift_ver = (s.height() - pic_theme.size().height()) / 2;
			pic.composite(pic_theme, composite_shift_hor, composite_shift_ver, AtopCompositeOp);
			
			// Save result
			pic.write((begins_with(filter["_filename_out"].as<std::string>(), "/") ? "" : path_to_task) + filter["_filename_out"].as<std::string>());
		}
		
		if (filter_name == "sigmoidal_contrast")
		{
			/* 
			 * _filename_in: ""
			 * _filename_out: ""
			 * contrast: 0.0
			 * grayscale: no
			 */
			
			// Take image
			
			Magick::Image pic;
			pic.read((begins_with(filter["_filename_in"].as<std::string>(), "/") ? "" : path_to_task) + filter["_filename_in"].as<std::string>());
			
			// Grayscale image
			if (filter["grayscale"] && filter["grayscale"].as<bool>()) pic.type(Magick::GrayscaleType);
			
			// Increase contrast
			double contrast = filter["contrast"] ? filter["contrast"].as<double>() : 0.0;
			//////////////// TODO contrast ////////////////////pic.sigmoidalContrast(1, contrast);
			
			// Save result
			pic.write((begins_with(filter["_filename_out"].as<std::string>(), "/") ? "" : path_to_task) + filter["_filename_out"].as<std::string>());
			
		}
		
		if (filter_name == "grayscale")
		{
			/* 
			 * _filename_in: ""
			 * _filename_out: ""
			 */
			
			// Take image
			Magick::Image pic;
			pic.read((begins_with(filter["_filename_in"].as<std::string>(), "/") ? "" : path_to_task) + filter["_filename_in"].as<std::string>());
			// Grayscale image
			pic.type(Magick::GrayscaleType);
			// Save result
			pic.write((begins_with(filter["_filename_out"].as<std::string>(), "/") ? "" : path_to_task) + filter["_filename_out"].as<std::string>());
		}
		
		if (filter_name == "faceshake")
		{
			/* 
			 * _filename_in: ""
			 * _filename_out: ""
			 * _frames_count: 0
			 * _animation_delay: 0
			 * _max_size: 0
			 * shake: no
			 * shake_coeff: 0.0
			 */
						
			// ============[ SCALE TO SAFE SIZE ]============
			
			// Read file from task, or by absolute path
			Magick::Image pic;
			pic.read((begins_with(filter["_filename_in"].as<std::string>(), "/") ? "" : path_to_task) + filter["_filename_in"].as<std::string>());
			// Resize image
			// Max width or height in pixels
			float new_pic_size = filter["_max_size"].as<float>();
			// resizing rectangle
			if (pic.size().width() > new_pic_size || pic.size().height() > new_pic_size) pic.resize(Magick::Geometry(new_pic_size, new_pic_size));
			// Write out safe input file
			pic.write((begins_with(filter["_filename_in"].as<std::string>(), "/") ? "" : path_to_task) + filter["_filename_in"].as<std::string>());
			
			// ============[ DETECT FACES ]============
			
			// Haar cascade location
			std::string face_cascade_name = path_to_binary + "resources/OpenCV/haarcascade_frontalface_alt.xml";
			// Create new haar object and load cascade
			cv::CascadeClassifier face_cascade;
			if (!face_cascade.load(face_cascade_name)) throw std::invalid_argument("OpenCV cascade " + face_cascade_name + " not found"); // Error
			
			// Read the image file
			cv::Mat frame;
			frame = cv::imread((begins_with(filter["_filename_in"].as<std::string>(), "/") ? "" : path_to_task) + filter["_filename_in"].as<std::string>());

			// Create a gray copy of image
			cv::Mat frame_gray;
			cv::cvtColor(frame, frame_gray, cv::COLOR_BGR2GRAY);
			cv::equalizeHist(frame_gray, frame_gray);

			// Detect faces
			std::vector<cv::Rect> faces;
			face_cascade.detectMultiScale(frame_gray, faces, 1.1, 2, 0 | cv::CASCADE_SCALE_IMAGE, cv::Size(30, 30));
			
			// Check, if faces found
			if (faces.size() < 1) throw krapiva::Exception_face("Faces not found"); // Error
			
			// Set Region of Interest
			cv::Rect biggest_face;
			int biggest_face_index;
			
			for (int i = 0; i < faces.size(); i++) // Iterate through all current elements (detected faces)
			{
				if (biggest_face.height * biggest_face.width < faces[i].height * faces[i].width)
				{
					biggest_face = faces[i];
					biggest_face_index = i;
				}
			}
			
			// SHOWING
			/*
			cv::Mat crop = frame(biggest_face);
				
			// Form a filename
			cv::imwrite("Biggest_face.png", crop);
			*/
			// SHOWING
			
			
			// ============[ CREATE ANIMATION ]============
			
			std::vector<Magick::Image> frames;
			
			
			const int face_distance_up = biggest_face.y;
			const int face_distance_down = pic.size().height() - biggest_face.y - biggest_face.height;
			const int face_distance_right = pic.size().width() - biggest_face.x - biggest_face.width;
			const int face_distance_left = biggest_face.x;
			const int n_frames = filter["_frames_count"].as<int>();
			
			Magick::Image current_frame;
			
			for (int i = 0; i < n_frames; ++i)
			{
				float current_frame_width  = biggest_face.width +  (face_distance_right + face_distance_left) * ((float)1 - (float)i / n_frames);
				float current_frame_height = biggest_face.height + (face_distance_up    + face_distance_down) * ((float)1 - (float)i / n_frames);
				
				Magick::Geometry current_rectangle(
					current_frame_width,
					current_frame_height,
					biggest_face.x * ((float)i / n_frames) + (filter["shake"] && filter["shake"].as<bool>() ? rand() % (int)(current_frame_width  * filter["shake_coeff"].as<float>()) : 0),
					biggest_face.y * ((float)i / n_frames) + (filter["shake"] && filter["shake"].as<bool>() ? rand() % (int)(current_frame_height * filter["shake_coeff"].as<float>()) : 0)
				);
				current_frame = pic;
				current_frame.crop(current_rectangle);
				// Resize frame
				current_frame.repage();
				scale_to_cover_image(pic.size(), current_frame);
				
				Magick::Geometry target_rectangle(
					pic.size().width(),
					pic.size().height(),
					current_frame.size().width() - pic.size().width(),
					current_frame.size().height() - pic.size().height()
				);
				
				current_frame.crop(target_rectangle);
				current_frame.repage();
				
				if (i == n_frames -1 && filter["_animation_delay_last"]) current_frame.animationDelay(filter["_animation_delay_last"].as<int>());
				else current_frame.animationDelay(filter["_animation_delay"].as<int>());
				
				frames.push_back(current_frame);
			}
			
			Magick::writeImages(frames.begin(), frames.end(), (begins_with(filter["_filename_out"].as<std::string>(), "/") ? "" : path_to_task) + filter["_filename_out"].as<std::string>());
		}

		if (filter_name == "shake")
		{
			/* 
			 * _filename_in: ""
			 * _filename_out: ""
			 * _frames_count: 0
			 * _animation_delay: 0
			 * _max_size: 0
			 * shake_coeff: 0.0
			 */
						
			// ============[ SCALE TO SAFE SIZE ]============
			
			// Read file from task, or by absolute path
			Magick::Image pic;
			pic.read((begins_with(filter["_filename_in"].as<std::string>(), "/") ? "" : path_to_task) + filter["_filename_in"].as<std::string>());
			// Resize image
			// Max width or height in pixels
			float new_pic_size = filter["_max_size"].as<float>();
			// resizing rectangle
			if (pic.size().width() > new_pic_size || pic.size().height() > new_pic_size) pic.resize(Magick::Geometry(new_pic_size, new_pic_size));
			
			
			// ============[ CREATE ANIMATION ]============
			
			std::vector<Magick::Image> frames;
			
			Magick::Image current_frame;
			
			const int n_frames = filter["_frames_count"].as<int>();
			
			for (int i = 0; i < n_frames; ++i)
			{
				Magick::Geometry current_rectangle(
					pic.size().width()  * ((float)1 - filter["shake_coeff"].as<float>()),
					pic.size().height() * ((float)1 - filter["shake_coeff"].as<float>()),
					rand() % (int)(pic.size().width()  * filter["shake_coeff"].as<float>()),
					rand() % (int)(pic.size().height() * filter["shake_coeff"].as<float>())
				);
				current_frame = pic;
				current_frame.crop(current_rectangle);
				// Resize frame
				current_frame.repage();
				scale_to_cover_image(pic.size(), current_frame);
				
				Magick::Geometry target_rectangle(
					pic.size().width(),
					pic.size().height(),
					current_frame.size().width() - pic.size().width(),
					current_frame.size().height() - pic.size().height()
				);
				
				current_frame.crop(target_rectangle);
				current_frame.repage();
				
				current_frame.animationDelay(filter["_animation_delay"].as<int>());
				frames.push_back(current_frame);
			}
			
			Magick::writeImages(frames.begin(), frames.end(), (begins_with(filter["_filename_out"].as<std::string>(), "/") ? "" : path_to_task) + filter["_filename_out"].as<std::string>());
		}
		
	} // <-- finish iterating through filters
	
	
}
