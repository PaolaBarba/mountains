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
#include "tif10_loader.h"
#include "tile.h"
#include "util.h"
#include "easylogging++.h"
/*
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
*/

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <cmath>

using std::string;

static const float TIF10_NODATA_ELEVATION = -32767.0f;

TIF10Loader::TIF10Loader(const FileFormat &format, int utmZone)
  : mFormat(format), mUtmZone(utmZone) {}

Tile *TIF10Loader::loadTile(const std::string &directory, double minLat, double minLng) {
  // Solo manejo de TIFF
  return loadFromTifFile(directory, minLat, minLng);
}

Tile *TIF10Loader::loadFromTifFile(const string &directory, double minLat, double minLng) {
  string filename = getTifFilename(minLat, minLng, mFormat);
  if (!directory.empty()) {
    filename = directory + "/" + filename;
  }

  FILE *infile = fopen(filename.c_str(), "rb");
  if (infile == nullptr) {
    std::cerr << "Failed to open file: " << filename << std::endl;
    return nullptr;
  }

  // Verificación de tamaño de imagen (suponiendo que sabes el tamaño esperado)
  const int tileSideLength = 10812;  // Cambia esto si es necesario
  const int tileSideHeight = 10812;  // Cambia esto si es necesario
  
  Elevation *samples = (Elevation *) malloc(sizeof(Elevation) * tileSideLength * tileSideHeight);
  if (samples == nullptr) {
    std::cerr << "Failed to allocate memory for samples." << std::endl;
    fclose(infile);
    return nullptr;
  }

  // Leer los datos del raster
  float *pafScanline = (float *) malloc(sizeof(float) * tileSideLength);
  if (pafScanline == nullptr) {
    std::cerr << "Failed to allocate memory for scanline buffer." << std::endl;
    free(samples);
    fclose(infile);
    return nullptr;
  }

  // Asumiendo que el archivo tiene datos de imagen en formato de fila (como un archivo TIFF)
  for (int i = 0; i < tileSideHeight; i++) {
    size_t readSize = fread(pafScanline, sizeof(float), tileSideLength, infile);
    if (readSize != tileSideLength) {
      std::cerr << "Failed to read row " << i << " from file." << std::endl;
      free(pafScanline);
      free(samples);
      fclose(infile);
      return nullptr;
    }

    for (int j = 0; j < tileSideLength; j++) {
      float sample = pafScanline[j];
      if (sample == TIF10_NODATA_ELEVATION) {
        samples[i * tileSideLength + j] = Tile::NODATA_ELEVATION;
      } else {
        samples[i * tileSideLength + j] = sample;
      }
    }
  }

  free(pafScanline);
  fclose(infile);

  Tile *tile = new Tile(tileSideLength, tileSideHeight, samples);
  return tile;
}

string TIF10Loader::getTifFilename(double minLat, double minLng, const FileFormat &format) {
  char buf[100];
  snprintf(buf, sizeof(buf), "%c%02d%c%03d.tif",
           (minLat >= 0) ? 'N' : 'S',
           abs(static_cast<int>(minLat)),
           (minLng >= 0) ? 'E' : 'W',
           abs(static_cast<int>(minLng)));
  return buf;
}

int TIF10Loader::fractionalDegree(double degree) const {
  double excess = fabs(degree - static_cast<int>(degree));
  return static_cast<int>(std::round(100 * excess));
}


/*
using std::string;

static const float TIF10_NODATA_ELEVATION = -32767.0f;

TIF10Loader::TIF10Loader(const FileFormat &format, int utmZone)
  : mFormat(format), mUtmZone(utmZone) {}

Tile *TIF10Loader::loadTile(const std::string &directory, double minLat, double minLng) {
    return loadFromTifFile(directory, minLat, minLng);
}

Tile *TIF10Loader::loadFromTifFile(const string &directory, double minLat, double minLng) {
    string filename = getTifFilename(minLat, minLng, mFormat);
    if (!directory.empty()) {
        filename = directory + "/" + filename;
    }
  std::cerr << "Attempting to open file: " << filename << std::endl;
std::cerr << "Opening file at: " << filename << std::endl;

    FILE *infile = fopen(filename.c_str(), "rb");
    if (infile == nullptr) {
		std::cerr << "Failed to open file: " << filename << std::endl;
        return nullptr;
    }

    // Aquí asumimos que el TIFF es un archivo binario simple con un solo bloque de datos
    // y sin compresión, y que los datos están almacenados en formato flotante de 32 bits.
    const int tileSideLength = 10812;  // Debes ajustar el tamaño según el TIFF real
    const int tileSideHeight = 10812;  // Debes ajustar el tamaño según el TIFF real
    float *samples = (float *) malloc(sizeof(float) * tileSideLength * tileSideHeight);

    if (samples == nullptr) {
        fclose(infile);
        return nullptr;
    }

    size_t numRead = fread(samples, sizeof(float), tileSideLength * tileSideHeight, infile);
    if (numRead != tileSideLength * tileSideHeight) {
        fprintf(stderr, "Failed to read the expected amount of data from %s\n", filename.c_str());
        free(samples);
        fclose(infile);
        return nullptr;
    }

    fclose(infile);

    // Reemplazar valores de NODATA
    for (int i = 0; i < tileSideLength * tileSideHeight; ++i) {
        if (samples[i] == TIF10_NODATA_ELEVATION) {
            samples[i] = Tile::NODATA_ELEVATION;
        }
    }

    Tile *tile = new Tile(tileSideLength, tileSideHeight, samples);
    free(samples);
    return tile;
}

string TIF10Loader::getTifFilename(double minLat, double minLng, const FileFormat &format) {
    char buf[100];
    snprintf(buf, sizeof(buf), "%c%02d%c%03d.tif",
             (minLat >= 0) ? 'N' : 'S',
             abs(static_cast<int>(minLat)),
             (minLng >= 0) ? 'E' : 'W',
             abs(static_cast<int>(minLng)));
    return buf;
}

int TIF10Loader::fractionalDegree(double degree) const {
    double excess = fabs(degree - static_cast<int>(degree));
    return static_cast<int>(std::round(100 * excess));
}
*/