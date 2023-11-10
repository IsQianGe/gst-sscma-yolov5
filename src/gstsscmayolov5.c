/*
 * GStreamer
 * Copyright (C) 2005 Thomas Vander Stichele <thomas@apestaart.org>
 * Copyright (C) 2005 Ronald S. Bultje <rbultje@ronald.bitfreak.net>
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

/**
 * SECTION:element-sscmayolov5
 *
 * FIXME:Describe sscmayolov5 here.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m fakesrc ! sscmayolov5 ! fakesink silent=TRUE
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gst/gst.h>

#include "gstsscmayolov5.h"
#include "tensor_info.h"

GST_DEBUG_CATEGORY_STATIC (gst_sscma_yolov5_debug);
#define GST_CAT_DEFAULT gst_sscma_yolov5_debug

/* Filter signals and args */
enum
{
  /* FILL ME */
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_SILENT
};

/* the capabilities of the inputs and outputs.
 *
 * describe the real formats here.
 */
static GstStaticPadTemplate sink_factory = GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("ANY")
    );

static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("ANY")
    );

#define gst_sscma_yolov5_parent_class parent_class
G_DEFINE_TYPE (GstSscmaYolov5, gst_sscma_yolov5, GST_TYPE_ELEMENT);

GST_ELEMENT_REGISTER_DEFINE (sscma_yolov5, "sscma_yolov5", GST_RANK_NONE,
    GST_TYPE_SSCMAYOLOV5);

static void gst_sscma_yolov5_set_property (GObject * object,
    guint prop_id, const GValue * value, GParamSpec * pspec);
static void gst_sscma_yolov5_get_property (GObject * object,
    guint prop_id, GValue * value, GParamSpec * pspec);

static gboolean gst_sscma_yolov5_sink_event (GstPad * pad,
    GstObject * parent, GstEvent * event);
static GstFlowReturn gst_sscma_yolov5_chain (GstPad * pad,
    GstObject * parent, GstBuffer * buf);

/* GObject vmethod implementations */

/* initialize the sscmayolov5's class */
static void
gst_sscma_yolov5_class_init (GstSscmaYolov5Class * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;
  GstBaseTransformClass *trans_class;

  trans_class = (GstBaseTransformClass *) klass;
  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;

  gobject_class->set_property = gst_sscma_yolov5_set_property;
  gobject_class->get_property = gst_sscma_yolov5_get_property;

  g_object_class_install_property (gobject_class, PROP_SILENT,
      g_param_spec_boolean ("silent", "Silent", "Produce verbose output ?",
          FALSE, G_PARAM_READWRITE));

  gst_element_class_set_details_simple (gstelement_class,
      "SscmaYolov5",
      "FIXME:Generic",
      "FIXME:Generic Template Element", "qian <<ruiqian.tang@seeed.org>>");

  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&src_factory));
  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&sink_factory));
}

/* initialize the new element
 * instantiate pads and add them to element
 * set pad callback functions
 * initialize instance structure
 */
static void
gst_sscma_yolov5_init (GstSscmaYolov5 * filter)
{
  filter->sinkpad = gst_pad_new_from_static_template (&sink_factory, "sink");
  gst_pad_set_event_function (filter->sinkpad,
      GST_DEBUG_FUNCPTR (gst_sscma_yolov5_sink_event));
  gst_pad_set_chain_function (filter->sinkpad,
      GST_DEBUG_FUNCPTR (gst_sscma_yolov5_chain));
  GST_PAD_SET_PROXY_CAPS (filter->sinkpad);
  gst_element_add_pad (GST_ELEMENT (filter), filter->sinkpad);

  filter->srcpad = gst_pad_new_from_static_template (&src_factory, "src");
  GST_PAD_SET_PROXY_CAPS (filter->srcpad);
  gst_element_add_pad (GST_ELEMENT (filter), filter->srcpad);

  filter->silent = FALSE;
}

/** @brief Handle "PROP_MODEL" for set-property */
static gint
_gtfc_setprop_MODEL (GstSscmaYolov5 * priv,
    GstSscmaYolov5Properties * prop, const GValue * value)
{
  gint status = 0;
  const gchar *model_files = g_value_get_string (value);
  GstSscmaYolov5Properties _prop;

  if (!model_files) {
    g_print ("Invalid model provided to the tensor-filter.");
    return 0;
  }
  _prop.model_files = NULL;

  if (prop->fw_opened) {
    /** Store a copy of the original prop in case the reload fails */
    memcpy (&_prop, prop, sizeof (GstSscmaYolov5Properties));
    _prop.model_files =
        (const gchar **) g_strdupv ((gchar **) prop->model_files);
  }

  g_strfreev_const (prop->model_files);

  if (model_files) {
    prop->model_files = (const gchar **) g_strsplit_set (model_files, ",", -1);
    prop->num_models = g_strv_length ((gchar **) prop->model_files);
  } else {
    prop->model_files = NULL;
    prop->num_models = 0;
  }
  return 0;
}

/** @brief Handle "PROP_INPUT" for set-property */
static gint
_gtfc_setprop_DIMENSION (GstSscmaYolov5 * priv,
    const GValue * value)
{
  GstSscmaYolov5Properties *prop;
  GstTensorsInfo *info;
  unsigned int *rank;
  int configured;

  prop = &priv->prop;
  info = &prop->input_meta;
  rank = prop->input_ranks;
  configured = prop->input_configured;

  if (!configured && value) {
    guint num_dims;
    gchar **str_dims;
    guint i;

    str_dims = g_strsplit_set (g_value_get_string (value), ",.", -1);
    num_dims = g_strv_length (str_dims);

    if (num_dims > NNS_TENSOR_SIZE_LIMIT + NNS_TENSOR_SIZE_EXTRA_LIMIT) {
      g_print ("Invalid param, dimensions (%d) max (%d)\n",
          num_dims, NNS_TENSOR_SIZE_LIMIT + NNS_TENSOR_SIZE_EXTRA_LIMIT);

      num_dims = NNS_TENSOR_SIZE_LIMIT + NNS_TENSOR_SIZE_EXTRA_LIMIT;
    }

    for (i = 0; i < num_dims; ++i) {
      rank[i] = gst_tensor_parse_dimension (str_dims[i],
          gst_tensors_info_get_nth_info (info, i)->dimension);
    }
    g_strfreev (str_dims);

    if (num_dims > 0) {
      if (info->num_tensors > 0 && info->num_tensors != num_dims) {
        g_print
            ("Invalid dimension, given param does not match with old value.");
      }

      info->num_tensors = num_dims;
    }
  } else if (value) {
    /** Once configured, it cannot be changed in runtime for now */
    g_print
        ("Cannot change dimension once the element/pipeline is configured.");
  }
  return 0;
}

/** @brief Handle "PROP_INPUTTYPE" and "PROP_OUTPUTTYPE" for set-property */
static gint
_gtfc_setprop_TYPE (GstSscmaYolov5 * priv,
    const GValue * value, const gboolean is_input)
{
  GstSscmaYolov5Properties *prop;
  GstTensorsInfo *info;
  int configured;

  prop = &priv->prop;

  if (is_input) {
    info = &prop->input_meta;
    configured = prop->input_configured;
  } else {
    info = &prop->output_meta;
    configured = prop->output_configured;
  }

  if (!configured && value) {
    guint num_types;

    num_types = gst_tensors_info_parse_types_string (info,
        g_value_get_string (value));

    if (num_types > 0) {
      if (info->num_tensors > 0 && info->num_tensors != num_types) {
        ml_logw ("Invalid type, given param does not match with old value.");
      }

      info->num_tensors = num_types;
    }
  } else if (value) {
    /** Once configured, it cannot be changed in runtime for now */
    ml_loge ("Cannot change type once the element/pipeline is configured.");
  }
  return 0;
}

static void
gst_sscma_yolov5_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstSscmaYolov5 *filter = GST_SSCMAYOLOV5 (object);
  GstSscmaYolov5Properties *filter;
  gchar *strval;
  UNUSED (pspec);

  prop = &filter->prop;
  switch (prop_id) {
    case PROP_SILENT:
      filter->silent = g_value_get_boolean (value);
      break;
    // 输入模型 mode=xxx,xxx（可为多个）
    case PROP_MODEL:
      status = _gtfc_setprop_MODEL (filter, prop, value);
      break;
    // 输入视频大小 input=320:320:3
    case PROP_INPUT:
      status = _gtfc_setprop_DIMENSION (filter, value);
      break;
    // 输入格式 inputformat=RGB
    case PROP_INPUTFORMAT:
      status = _gtfc_setprop_FORMAT (filter, value);
      break;
    // 输出类型 inputtype=float32
    case PROP_INPUTTYPE:
      status = _gtfc_setprop_TYPE (filter, value, TRUE);
      break;
    // 输出类型 outputtype=float32
    case PROP_OUTPUTTYPE:
      status = _gtfc_setprop_TYPE (filter, value, FALSE);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_sscma_yolov5_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GstSscmaYolov5 *filter = GST_SSCMAYOLOV5 (object);

  switch (prop_id) {
    case PROP_SILENT:
      g_value_set_boolean (value, filter->silent);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

/* GstElement vmethod implementations */

/* this function handles sink events */
static gboolean
gst_sscma_yolov5_sink_event (GstPad * pad, GstObject * parent,
    GstEvent * event)
{
  GstSscmaYolov5 *filter;
  gboolean ret;

  filter = GST_SSCMAYOLOV5 (parent);

  GST_LOG_OBJECT (filter, "Received %s event: %" GST_PTR_FORMAT,
      GST_EVENT_TYPE_NAME (event), event);

  switch (GST_EVENT_TYPE (event)) {
    case GST_EVENT_CAPS:
    {
      GstCaps *caps;

      gst_event_parse_caps (event, &caps);
      /* do something with the caps */

      /* and forward */
      ret = gst_pad_event_default (pad, parent, event);
      break;
    }
    default:
      ret = gst_pad_event_default (pad, parent, event);
      break;
  }
  return ret;
}

/* chain function
 * this function does the actual processing
 */
static GstFlowReturn
gst_sscma_yolov5_chain (GstPad * pad, GstObject * parent, GstBuffer * buf)
{
  GstSscmaYolov5 *filter;

  filter = GST_SSCMAYOLOV5 (parent);

  if (filter->silent == FALSE)
    g_print ("I'm plugged, therefore I'm in.\n");

  /* just push out the incoming buffer without touching it */
  return gst_pad_push (filter->srcpad, buf);
}


/* entry point to initialize the plug-in
 * initialize the plug-in itself
 * register the element factories and other features
 */
static gboolean
sscmayolov5_init (GstPlugin * sscmayolov5)
{
  /* debug category for filtering log messages
   *
   * exchange the string 'Template sscmayolov5' with your description
   */
  GST_DEBUG_CATEGORY_INIT (gst_sscma_yolov5_debug, "sscmayolov5",
      0, "Template sscmayolov5");

  return GST_ELEMENT_REGISTER (sscma_yolov5, sscmayolov5);
}

/* PACKAGE: this is usually set by meson depending on some _INIT macro
 * in meson.build and then written into and defined in config.h, but we can
 * just set it ourselves here in case someone doesn't use meson to
 * compile this code. GST_PLUGIN_DEFINE needs PACKAGE to be defined.
 */
#ifndef PACKAGE
#define PACKAGE "myfirstsscmayolov5"
#endif

/* gstreamer looks for this structure to register sscmayolov5s
 *
 * exchange the string 'Template sscmayolov5' with your sscmayolov5 description
 */
GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    sscmayolov5,
    "sscma_yolov5",
    sscmayolov5_init,
    PACKAGE_VERSION, GST_LICENSE, GST_PACKAGE_NAME, GST_PACKAGE_ORIGIN)
