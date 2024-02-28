/* 
 *  Copyright (c) 2017  Rafael Pagés
 *
 *  This file is part of SSMVtex
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 *  of the Software, and to permit persons to whom the Software is furnished to do
 *  so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 *  FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 *  COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 *  IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <cstring>
#include <cassert>

#include "image.h"

Image::Image(){
}

Image::Image(const std::string& _fileName){

	if(!this->imageFile_.load(_fileName.c_str())){
		std::cerr << "Image " << _fileName << " could not be read" << std::endl;
		std::cerr << "Filename length " << _fileName.length() << std::endl;
	}

	width_ = imageFile_.getWidth();
	height_ = imageFile_.getHeight();
    name_ = _fileName;
}

Image::Image(unsigned int _height, unsigned int _width, Color _background){

    width_ = _width;
    height_ = _height;

    fipImage tmp(FIT_BITMAP,width_,height_, 24);

    RGBQUAD color;
    color.rgbBlue = _background.getBlue();
    color.rgbGreen = _background.getGreen();
    color.rgbRed = _background.getRed();
    FreeImage_FillBackground(tmp, &color);

    imageFile_ = tmp;

}

Color Image::getColor (unsigned int _row, unsigned int _column) const{

    assert(_row < height_);
    assert(_column < width_);


    RGBQUAD col;
    if(!imageFile_.getPixelColor(_column,_row, &col)){
        std::cerr << _column << " " << _row << std::endl;
        std::cerr << name_ << std::endl;
    }
    Color color ((float)col.rgbRed, (float)col.rgbGreen, (float)col.rgbBlue);
    return color;

}

Color Image::interpolate (float _row, float _column, InterpolateMode _mode) const {

	const float c = _column - 0.5;
    const float r = _row - 0.5;
    int r_base = floor(r);
    int c_base = floor(c);

    const float x = r - (float)r_base;
    const float y = c - (float)c_base;

    r_base = std::max(r_base, 0);
    c_base = std::max(c_base, 0);

    Color final;
    Color A,B,C,D;

    // Bilinear
    if (_mode == BILINEAR){

        if (r_base + 1 >= (int) height_ || c_base + 1 >= (int) width_){
            // We are in the edge of an image, then we cannot interpolate
            final = getColor(r_base, c_base);
        } else {

           A = getColor(r_base, c_base);         // f(0,0)
           B = getColor(r_base + 1, c_base);     // f(1,0)
           C = getColor(r_base, c_base + 1);     // f(0,1)
           D = getColor(r_base + 1, c_base + 1); // f(1,1)

           // Bilinear interpolation: 
           // f(x,y) = f(0,0)(1-x)(1-y) + f(1,0)(x)(1-y) + f(0,1)(1-x)(y) + f(1,1)(x)(y)
           final = A + (B-A) * x + (C-A) * y + (A+D-B-C) * x * y;

 
        }
    
    } else if (_mode == BICUBIC){

    // Bi-Cubic

    	Color row_temp [4];

        if (r_base + 3 >= (int) height_ || r_base - 1 < 0|| c_base + 2 >= (int) width_ || c_base - 1 < 0){

            final = getColor(r_base, c_base);

        } else {

            for (int i = -1; i < 3; i++) {

                const Color M = getColor(r_base + i, c_base - 1);
                const Color N = getColor(r_base + i, c_base);
                const Color O = getColor(r_base + i, c_base + 1);
                const Color P = getColor(r_base + i, c_base + 2);

                A = P - O;
                B = N - M;
                C = O - M;

                row_temp[i + 1] = N + ( ( (A+B) * y - A - B - B) * y + C) * y;
            }

            A = row_temp[3] - row_temp[2];
            B = row_temp[1] - row_temp[0];
            C = row_temp[2] - row_temp[0];

            final = row_temp[1] + ( ( (A+B) * x - A - B - B) * x + C) * x;
        }

    } else {
    	std::cerr << "Mode " << _mode << " is unknown" << std::endl;
    	final = Color(0,0,0);
    }
    return final;
}


void Image::setColor (const Color& _color, unsigned int _row, unsigned int _column){
    
    RGBQUAD col;
    col.rgbBlue = _color.getBlue();
    col.rgbGreen = _color.getGreen();
    col.rgbRed = _color.getRed();

    imageFile_.setPixelColor(_column,_row,&col);
}

void Image::save(const std::string& _fileName){

    char * name = new char[_fileName.length()+1];
    strcpy(name, _fileName.c_str());

    this->imageFile_.convertTo24Bits();
    this->imageFile_.save(name, JPEG_QUALITYSUPERB);
}
