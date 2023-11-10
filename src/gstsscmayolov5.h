/*
 * GStreamer
 * Copyright (C) 2005 Thomas Vander Stichele <thomas@apestaart.org>
 * Copyright (C) 2005 Ronald S. Bultje <rbultje@ronald.bitfreak.net>
 * Copyright (C) 2020 Niels De Graef <niels.degraef@gmail.com>
 * Copyright (C) 2023 qian <<user@hostname.org>>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Alternatively, the contents of this file may be used under the
 * GNU Lesser General Public License Version 2.1 (the "LGPL"), in
 * which case the following provisions apply instead of the ones
 * mentioned above:
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __GST_SSCMAYOLOV5_H__
#define __GST_SSCMAYOLOV5_H__

#include <gst/gst.h>

G_BEGIN_DECLS

#define GST_TYPE_SSCMAYOLOV5 (gst_sscma_yolov5_get_type())
G_DECLARE_FINAL_TYPE (GstSscmaYolov5, gst_sscma_yolov5,
    GST, SSCMAYOLOV5, GstElement)

typedef struct _GstSscmaYolov5 GstSscmaYolov5;
/**
 * @brief GstMode's properties for NN framework (internal data structure)
 *
 * Because custom filters of tensor_filter may need to access internal data
 * of GstTensorFilter, we define this data structure here.
 */
typedef struct _GstSscmaYolov5Properties
{
  const char **model_files; /**< Filepath to the model file (as an argument for NNFW). char instead of gchar for non-glib custom plugins */
  int num_models; /**< number of model files. Some frameworks need multiple model files to initialize the graph (caffe, caffe2) */

  int input_configured; /**< TRUE if input tensor is configured. Use int instead of gboolean because this is refered by custom plugins. */
  GstTensorsInfo input_meta; /**< configured input tensor info */
  tensors_layout input_layout; /**< data layout info provided as a property to tensor_filter for the input, defaults to _NNS_LAYOUT_ANY for all the tensors */
  unsigned int input_ranks[NNS_TENSOR_SIZE_LIMIT + NNS_TENSOR_SIZE_EXTRA_LIMIT];  /**< the rank list of input tensors, it is calculated based on the dimension string. */

  int output_configured; /**< TRUE if output tensor is configured. Use int instead of gboolean because this is refered by custom plugins. */
  GstTensorsInfo output_meta; /**< configured output tensor info */
  tensors_layout output_layout; /**< data layout info provided as a property to tensor_filter for the output, defaults to _NNS_LAYOUT_ANY for all the tensors */
  unsigned int output_ranks[NNS_TENSOR_SIZE_LIMIT + NNS_TENSOR_SIZE_EXTRA_LIMIT];  /**< the rank list of output tensors, it is calculated based on the dimension string. */

  const char *custom_properties; /**< sub-plugin specific custom property values in string */
  accl_hw *hw_list; /**< accelerators supported by framework intersected with user provided accelerator preference, use in GstTensorFilterFramework V1 only */
  int num_hw;       /**< number of hardare accelerators in the hw_list supported by the framework */
  const char *accl_str; /**< accelerator configuration passed in as parameter, use in GstTensorFilterFramework V0 only */
  char *shared_tensor_filter_key; /**< the shared instance key to use same model representation */

} GstSscmaYolov5Properties;


struct _GstSscmaYolov5
{
  GstElement element;

  GstPad *sinkpad, *srcpad;

  gboolean silent;

  GstSscmaYolov5Properties prop; /**< NNFW plugin's properties */
};

G_END_DECLS

#endif /* __GST_SSCMAYOLOV5_H__ */
