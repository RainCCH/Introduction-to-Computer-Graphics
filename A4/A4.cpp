// Termm--Fall 2020

#include <glm/ext.hpp>

#include "A4.hpp"

void flip_image(Image & image) {
	size_t h = image.height();
	size_t w = image.width();
	for (size_t y = 0; y < h / 2; ++y) {
		for (size_t x = 0; x < w; ++x) {
			for (size_t c = 0; c < 3; ++c) {
				double temp = image(x, y, c);
				image(x, y, c) = image(x, h - y - 1, c);
				image(x, h - y - 1, c) = temp;
			}
		}
	}
}

void A4_Render(
		// What to render  
		SceneNode * root,

		// Image to write to, set to a given width and height  
		Image & image,

		// Viewing parameters  
		const glm::vec3 & eye,
		const glm::vec3 & view,
		const glm::vec3 & up,
		double fovy,

		// Lighting parameters  
		const glm::vec3 & ambient,
		const std::list<Light *> & lights
) {

  // Fill in raytracing code here...  

  std::cout << "F20: Calling A4_Render(\n" <<
		  "\t" << *root <<
          "\t" << "Image(width:" << image.width() << ", height:" << image.height() << ")\n"
          "\t" << "eye:  " << glm::to_string(eye) << std::endl <<
		  "\t" << "view: " << glm::to_string(view) << std::endl <<
		  "\t" << "up:   " << glm::to_string(up) << std::endl <<
		  "\t" << "fovy: " << fovy << std::endl <<
          "\t" << "ambient: " << glm::to_string(ambient) << std::endl <<
		  "\t" << "lights{" << std::endl;

	for(const Light * light : lights) {
		std::cout << "\t\t" <<  *light << std::endl;
	}
	std::cout << "\t}" << std::endl;
	std:: cout <<")" << std::endl;

	size_t h = image.height();
	size_t w = image.width();
	double aspect = (double)w / (double)h;
	Camera camera(eye, view, up, fovy, aspect);

	// for (uint y = 0; y < h; ++y) {
	// 	for (uint x = 0; x < w; ++x) {
	// 		// // Red: 
	// 		// image(x, y, 0) = (double)1.0;
	// 		// // Green: 
	// 		// image(x, y, 1) = (double)1.0;
	// 		// // Blue: 
	// 		// image(x, y, 2) = (double)1.0;
	// 		camera.render(image, lights, root, ambient);
	// 	}
	// }
	camera.render(image, lights, root, ambient);
	// Turn the image upside down
	flip_image(image);
}
