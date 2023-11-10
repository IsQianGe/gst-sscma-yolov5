#ifndef __GST_TENSOE_INFO_H__
#define __GST_TENSOE_INFO_H__

#include <gst/gst.h>

typedef uint32_t tensor_dim[NNS_TENSOR_RANK_LIMIT];

/**
 * @brief Possible data element types of other/tensor.
 */
typedef enum _tensor_type
{
  _TENOR_INT32 = 0,
  _TENOR_UINT32,
  _TENOR_INT16,
  _TENOR_UINT16,
  _TENOR_INT8,
  _TENOR_UINT8,
  _TENOR_FLOAT64,
  _TENOR_FLOAT32,
  _TENOR_INT64,
  _TENOR_UINT64,
  _TENOR_FLOAT16, /**< added with nnstreamer 2.1.1-devel. If you add any operators (e.g., tensor_transform) to float16, it will either be not supported or be too inefficient. */

  _TENOR_END,
} tensor_type;

/**
 * @brief Internal meta data exchange format for a other/tensors instance
 * @note This must be coherent with api/capi/include/nnstreamer-capi-private.h:ml_tensors_info_s
 */
typedef struct
{
  unsigned int num_tensors; /**< The number of tensors */
  GstTensorInfo info[NNS_TENSOR_SIZE_LIMIT]; /**< The list of tensor info */
  GstTensorInfo *extra; /**< The list of tensor info for tensors whose idx is larger than NNS_TENSOR_SIZE_LIMIT */
  tensor_format format; /**< tensor stream type */
} GstTensorsInfo;

void gst_tensor_info_init (GstTensorInfo * info);
void gst_tensor_info_free (GstTensorInfo * info);

GstTensorInfo * gst_tensors_info_get_nth_info (GstTensorsInfo * info, guint nth);
gboolean gst_tensors_info_extra_create (GstTensorsInfo * info);
void gst_tensors_info_extra_free (GstTensorsInfo * info);
guint gst_tensors_info_parse_types_string (GstTensorsInfo * info, const gchar * type_string);
guint gst_tensor_parse_dimension (const gchar * dimstr, tensor_dim dim);

#endif /* __GST_TENSOE_INFO_H__ */