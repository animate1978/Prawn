
#ifndef fltk_tiff_Image_h
#define fltk_tiff_Image_h

//#include "SharedImage.h"
#include <fltk/SharedImage.h>

namespace fltk {

class FL_IMAGES_API tiffImage : public SharedImage {
public:
  tiffImage(const char* filename);
  // virtual function overrides
  bool fetch();
};

}

#endif

