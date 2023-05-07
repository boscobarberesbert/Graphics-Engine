#pragma once

#include "engine.h"

// OpenGL/VRML Materials
// These numbers come from the OpenGL teapots.c demo, � Silicon Graphics, Inc., � 1994, Mark J. Kilgard.
// http://devernay.free.fr/cours/opengl/materials.html

void SetupDefaultMaterials(App* app)
{
	//		 Name							 Ambient								Diffuse										 Specular					Shininess
	Material emerald		(vec3(0.0215f,	 0.1745f,	0.0215f),	vec3(0.07568f,	0.61424f,	 0.07568f),	   vec3(0.633f,		 0.727811f,	  0.633f),		0.6f);		  app->materials.push_back(emerald);	   app->materialIndexes.insert(std::make_pair("emerald",		app->materials.size() - 1));
	Material jade			(vec3(0.135f,	 0.2225f,	0.1575f),	vec3(0.54f,		0.89f,		 0.63f),	   vec3(0.316228f,	 0.316228f,	  0.316228f),	0.1f);		  app->materials.push_back(jade);		   app->materialIndexes.insert(std::make_pair("jade",			app->materials.size() - 1));
	Material obsidian		(vec3(0.05375f,	 0.05f,		0.06625f),	vec3(0.18275f,	0.17f,		 0.22525f),	   vec3(0.332741f,	 0.328634f,	  0.346435f),	0.3f);		  app->materials.push_back(obsidian);	   app->materialIndexes.insert(std::make_pair("obsidian",		app->materials.size() - 1));
	Material pearl			(vec3(0.25f,	 0.20725f,	0.20725f),	vec3(1.0f,		0.829f,		 0.829f),	   vec3(0.296648f,	 0.296648f,	  0.296648f),	0.088f);	  app->materials.push_back(pearl);		   app->materialIndexes.insert(std::make_pair("pearl",			app->materials.size() - 1));
	Material ruby			(vec3(0.1745f,	 0.01175f,	0.01175f),	vec3(0.61424f,	0.04136f,	 0.04136f),	   vec3(0.727811f,	 0.626959f,	  0.626959f),	0.6f);		  app->materials.push_back(ruby);		   app->materialIndexes.insert(std::make_pair("ruby",			app->materials.size() - 1));
	Material turquoise		(vec3(0.1f,		 0.18725f,	0.1745f),	vec3(0.396f,	0.74151f,	 0.69102f),	   vec3(0.297254f,	 0.30829f,	  0.306678f),	0.1f);		  app->materials.push_back(turquoise);	   app->materialIndexes.insert(std::make_pair("turquoise",		app->materials.size() - 1));
	Material brass			(vec3(0.329412f, 0.223529f, 0.027451f), vec3(0.780392f, 0.568627f,	 0.113725f),   vec3(0.992157f,	 0.941176f,	  0.807843f),	0.21794872f); app->materials.push_back(brass);		   app->materialIndexes.insert(std::make_pair("brass",			app->materials.size() - 1));
	Material bronze			(vec3(0.2125f,	 0.1275f,	0.054f),	vec3(0.714f,	0.4284f,	 0.18144f),	   vec3(0.393548f,	 0.271906f,	  0.166721f),	0.2f);		  app->materials.push_back(bronze);		   app->materialIndexes.insert(std::make_pair("bronze",			app->materials.size() - 1));
	Material chrome			(vec3(0.25f,	 0.25f,		0.25f),		vec3(0.4f,		0.4f,		 0.4f),		   vec3(0.774597f,	 0.774597f,	  0.774597f),	0.6f);		  app->materials.push_back(chrome);		   app->materialIndexes.insert(std::make_pair("chrome",			app->materials.size() - 1));
	Material copper			(vec3(0.19125f,	 0.0735f,	0.0225f),	vec3(0.7038f,	0.27048f,	 0.0828f),	   vec3(0.256777f,	 0.137622f,	  0.086014f),	0.1f);		  app->materials.push_back(copper);		   app->materialIndexes.insert(std::make_pair("copper",			app->materials.size() - 1));
	Material gold			(vec3(0.24725f,	 0.1995f,	0.0745f),	vec3(0.75164f,	0.60648f,	 0.22648f),	   vec3(0.628281f,	 0.555802f,	  0.366065f),	0.4f);		  app->materials.push_back(gold);		   app->materialIndexes.insert(std::make_pair("gold",			app->materials.size() - 1));
	Material silver			(vec3(0.19225f,	 0.19225f,	0.19225f),	vec3(0.50754f,	0.50754f,	 0.50754f),	   vec3(0.508273f,	 0.508273f,	  0.508273f),	0.4f);		  app->materials.push_back(silver);		   app->materialIndexes.insert(std::make_pair("silver",			app->materials.size() - 1));
	Material blackPlastic	(vec3(0.0f,		 0.0f,		0.0f),		vec3(0.01f,		0.01f,		 0.01f),	   vec3(0.50f,		 0.50f,		  0.50f),		0.25f);		  app->materials.push_back(blackPlastic);  app->materialIndexes.insert(std::make_pair("black plastic",	app->materials.size() - 1));
	Material cyanPlastic	(vec3(0.0f,		 0.1f,		0.06f),		vec3(0.0f,		0.50980392f, 0.50980392f), vec3(0.50196078f, 0.50196078f, 0.50196078f), 0.25f);		  app->materials.push_back(cyanPlastic);   app->materialIndexes.insert(std::make_pair("cyan plastic",	app->materials.size() - 1));
	Material greenPlastic	(vec3(0.0f,		 0.0f,		0.0f),		vec3(0.1f,		0.35f,		 0.1f),		   vec3(0.45f,		 0.55f,		  0.45f),		0.25f);		  app->materials.push_back(greenPlastic);  app->materialIndexes.insert(std::make_pair("green plastic",	app->materials.size() - 1));
	Material redPlastic		(vec3(0.0f,		 0.0f,		0.0f),		vec3(0.5f,		0.0f,		 0.0f),		   vec3(0.7f,		 0.6f,		  0.6f),		0.25f);		  app->materials.push_back(redPlastic);	   app->materialIndexes.insert(std::make_pair("red plastic",	app->materials.size() - 1));
	Material whitePlastic	(vec3(0.0f,		 0.0f,		0.0f),		vec3(0.55f,		0.55f,		 0.55f),	   vec3(0.70f,		 0.70f,		  0.70f),		0.25f);		  app->materials.push_back(whitePlastic);  app->materialIndexes.insert(std::make_pair("white plastic",	app->materials.size() - 1));
	Material yellowPlastic	(vec3(0.0f,		 0.0f,		0.0f),		vec3(0.5f,		0.5f,		 0.0f),		   vec3(0.60f,		 0.60f,		  0.50f),		0.25f);		  app->materials.push_back(yellowPlastic); app->materialIndexes.insert(std::make_pair("yellow plastic", app->materials.size() - 1));
	Material blackRubber	(vec3(0.02f,	 0.02f,		0.02f),		vec3(0.01f,		0.01f,		 0.01f),	   vec3(0.4f,		 0.4f,		  0.4f),		0.078125f);	  app->materials.push_back(blackRubber);   app->materialIndexes.insert(std::make_pair("black rubber",	app->materials.size() - 1));
	Material cyanRubber		(vec3(0.0f,		 0.05f,		0.05f),		vec3(0.4f,		0.5f,		 0.5f),		   vec3(0.04f,		 0.7f,		  0.7f),		0.078125f);	  app->materials.push_back(cyanRubber);	   app->materialIndexes.insert(std::make_pair("cyan rubber",	app->materials.size() - 1));
	Material greenRubber	(vec3(0.0f,		 0.05f,		0.0f),		vec3(0.4f,		0.5f,		 0.4f),		   vec3(0.04f,		 0.7f,		  0.04f),		0.078125f);	  app->materials.push_back(greenRubber);   app->materialIndexes.insert(std::make_pair("green rubber",	app->materials.size() - 1));
	Material redRubber		(vec3(0.05f,	 0.0f,		0.0f),		vec3(0.5f,		0.4f,		 0.4f),		   vec3(0.7f,		 0.04f,		  0.04f),		0.078125f);	  app->materials.push_back(redRubber);	   app->materialIndexes.insert(std::make_pair("red rubber",		app->materials.size() - 1));
	Material whiteRubber	(vec3(0.05f,	 0.05f,		0.05f),		vec3(0.5f,		0.5f,		 0.5f),		   vec3(0.7f,		 0.7f,		  0.7f),		0.078125f);	  app->materials.push_back(whiteRubber);   app->materialIndexes.insert(std::make_pair("white rubber",	app->materials.size() - 1));
	Material yellowRubber	(vec3(0.05f,	 0.05f,		0.0f),		vec3(0.5f,		0.5f,		 0.4f),		   vec3(0.7f,		 0.7f,		  0.04f),		0.078125f);	  app->materials.push_back(yellowRubber);  app->materialIndexes.insert(std::make_pair("yellow rubber",	app->materials.size() - 1));
}
