#include "tensor_info.h"

/**
 * @brief Initialize the tensor info structure
 * @param info tensor info structure to be initialized
 */
void
gst_tensor_info_init (GstTensorInfo * info)
{
  guint i;

  g_return_if_fail (info != NULL);

  info->name = NULL;
  info->type = _TENOR_END;

  for (i = 0; i < NNS_TENSOR_RANK_LIMIT; i++) {
    info->dimension[i] = 0;
  }
}

/**
 * @brief Free allocated data in tensor info structure
 * @param info tensor info structure
 */
void
gst_tensor_info_free (GstTensorInfo * info)
{
  g_return_if_fail (info != NULL);

  if (info->name) {
    g_free (info->name);
    info->name = NULL;
  }
}

/**
 * @brief Get the pointer of nth tensor information.
 */
GstTensorInfo *
gst_tensors_info_get_nth_info (GstTensorsInfo * info, guint nth)
{
  g_return_val_if_fail (info != NULL, NULL);

  if (nth < NNS_TENSOR_SIZE_LIMIT)
    return &info->info[nth];

  if (!gst_tensors_info_extra_create (info))
    return NULL;

  if (nth < NNS_TENSOR_SIZE_LIMIT + NNS_TENSOR_SIZE_EXTRA_LIMIT)
    return &info->extra[nth - NNS_TENSOR_SIZE_LIMIT];

  g_print ("Failed to get the information, invalid index %u.", nth);
  return NULL;
}

/**
 * @brief Allocate and initialize the extra info in given tensors info.
 * @param[in,out] info tensors info to be updated.
 */
gboolean
gst_tensors_info_extra_create (GstTensorsInfo * info)
{
  GstTensorInfo *new;
  guint i;

  g_return_val_if_fail (info != NULL, FALSE);

  if (info->extra) {
    return TRUE;
  }

  new = g_try_new0 (GstTensorInfo, NNS_TENSOR_SIZE_EXTRA_LIMIT);
  if (!new) {
    g_print ("Failed to allocate memory for extra tensors info");
    return FALSE;
  }

  for (i = 0; i < NNS_TENSOR_SIZE_EXTRA_LIMIT; ++i) {
    gst_tensor_info_init (&new[i]);
  }

  info->extra = new;

  return TRUE;
}

/**
 * @brief Free allocated extra info in given tensors info.
 * @param[in,out] info tensors info whose extra info is to be freed.
 */
void
gst_tensors_info_extra_free (GstTensorsInfo * info)
{
  guint i;

  g_return_if_fail (info != NULL);

  if (info->extra) {
    for (i = 0; i < NNS_TENSOR_SIZE_EXTRA_LIMIT; ++i)
      gst_tensor_info_free (&info->extra[i]);

    g_free (info->extra);
    info->extra = NULL;
  }
}

/**
 * @brief Parse the string of types
 * @param info tensors info structure
 * @param type_string string of types
 * @return number of parsed types
 */
guint
gst_tensors_info_parse_types_string (GstTensorsInfo * info,
    const gchar * type_string)
{
  guint num_types = 0;
  GstTensorInfo *_info;

  g_return_val_if_fail (info != NULL, 0);

  if (type_string) {
    guint i;
    gchar **str_types;

    str_types = g_strsplit_set (type_string, ",.", -1);
    num_types = g_strv_length (str_types);

    if (num_types > NNS_TENSOR_SIZE_LIMIT + NNS_TENSOR_SIZE_EXTRA_LIMIT) {
      nns_logw ("Invalid param, types (%d) max (%d)\n",
          num_types, NNS_TENSOR_SIZE_LIMIT + NNS_TENSOR_SIZE_EXTRA_LIMIT);

      num_types = NNS_TENSOR_SIZE_LIMIT + NNS_TENSOR_SIZE_EXTRA_LIMIT;
    }

    if (num_types >= NNS_TENSOR_SIZE_LIMIT)
      gst_tensors_info_extra_create (info);

    for (i = 0; i < num_types; i++) {
      _info = gst_tensors_info_get_nth_info (info, i);
      _info->type = gst_tensor_get_type (str_types[i]);
    }

    g_strfreev (str_types);
  }

  return num_types;
}

/**
 * @brief Parse tensor dimension parameter string
 * @return The Rank. 0 if error.
 * @param dimstr The dimension string in the format of d1:...:d16, d1:d2:d3, d1:d2, or d1, where dN is a positive integer and d1 is the innermost dimension; i.e., dim[d16]...[d1];
 * @param dim dimension to be filled.
 */
guint
gst_tensor_parse_dimension (const gchar * dimstr, tensor_dim dim)
{
  guint rank = 0;
  guint64 val;
  gchar **strv;
  gchar *dim_string;
  guint i, num_dims;

  /* 0-init */
  for (i = 0; i < NNS_TENSOR_RANK_LIMIT; i++)
    dim[i] = 0;

  if (dimstr == NULL)
    return 0;

  /* remove spaces */
  dim_string = g_strdup (dimstr);
  g_strstrip (dim_string);

  strv = g_strsplit (dim_string, ":", NNS_TENSOR_RANK_LIMIT);
  num_dims = g_strv_length (strv);

  for (i = 0; i < num_dims; i++) {
    g_strstrip (strv[i]);
    if (strv[i] == NULL || strlen (strv[i]) == 0)
      break;

    val = g_ascii_strtoull (strv[i], NULL, 10);
    dim[i] = (uint32_t) val;
    rank = i + 1;
  }

  g_strfreev (strv);
  g_free (dim_string);
  return rank;
}

/**
 * @brief Get tensor type from string input.
 * @return Corresponding tensor_type. _TENOR_END if unrecognized value is there.
 * @param typestr The string type name, supposed to be one of tensor_element_typename[]
 */
tensor_type
gst_tensor_get_type (const gchar * typestr)
{
  gsize size, len;
  gchar *type_string;
  tensor_type type = _TENOR_END;

  if (typestr == NULL)
    return _TENOR_END;

  /* remove spaces */
  type_string = g_strdup (typestr);
  g_strstrip (type_string);

  len = strlen (type_string);

  if (len == 0) {
    g_free (type_string);
    return _TENOR_END;
  }

  if (g_regex_match_simple ("^uint(8|16|32|64)$",
          type_string, G_REGEX_CASELESS, 0)) {
    size = (gsize) g_ascii_strtoull (&type_string[4], NULL, 10);

    switch (size) {
      case 8:
        type = _TENOR_UINT8;
        break;
      case 16:
        type = _TENOR_UINT16;
        break;
      case 32:
        type = _TENOR_UINT32;
        break;
      case 64:
        type = _TENOR_UINT64;
    }
  } else if (g_regex_match_simple ("^int(8|16|32|64)$",
          type_string, G_REGEX_CASELESS, 0)) {
    size = (gsize) g_ascii_strtoull (&type_string[3], NULL, 10);

    switch (size) {
      case 8:
        type = _TENOR_INT8;
        break;
      case 16:
        type = _TENOR_INT16;
        break;
      case 32:
        type = _TENOR_INT32;
        break;
      case 64:
        type = _TENOR_INT64;
    }
  } else if (g_regex_match_simple ("^float(16|32|64)$",
          type_string, G_REGEX_CASELESS, 0)) {
    size = (gsize) g_ascii_strtoull (&type_string[5], NULL, 10);

    switch (size) {
      case 16:
        type = _TENOR_FLOAT16;
        break;
      case 32:
        type = _TENOR_FLOAT32;
        break;
      case 64:
        type = _TENOR_FLOAT64;
    }
  }

  g_free (type_string);
  return type;
}