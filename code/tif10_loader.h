/*
 * MIT License
 * 
 * Copyright (c) 2017 Andrew Kirmse
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef _TIF10_LOADER_H_
#define _TIF10_LOADER_H_

#include "util.h"
#include "easylogging++.h"
#include "tile_loader.h"
#include "tile.h"
#include "file_format.h"  // Asegúrate de que este archivo existe y tiene la definición de FileFormat
#include <string>

class TIF10Loader : public TileLoader {
public:
  // Constructor para inicializar con un formato y una zona UTM (si es necesario)
  explicit TIF10Loader(const FileFormat &format, int utmZone);

  // Método para cargar un tile TIFF
  virtual Tile *loadTile(const std::string &directory, double minLat, double minLng);

private:
  FileFormat mFormat;
  int mUtmZone;  // Para datos en coordenadas UTM, si es necesario
  
  // Método para cargar el tile desde un archivo TIFF
  Tile *loadFromTifFile(const std::string &directory, double minLat, double minLng);
  
  // Método para obtener el nombre del archivo TIFF para las coordenadas dadas
  std::string getTifFilename(double minLat, double minLng, const FileFormat &format);

  // Método opcional: devuelve los centésimos de grado del valor dado, si es relevante para TIFF
  int fractionalDegree(double degree) const;
};

#endif  // _TIF10_LOADER_H_