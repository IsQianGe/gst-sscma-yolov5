#include "tensor_info.h"


/**
 * @brief Initialize the tensors layout.
 */
void
gst_tensors_layout_init (tensors_layout layout)
{
  int i;

  for (i = 0; i < NNS_TENSOR_SIZE_LIMIT + NNS_TENSOR_SIZE_EXTRA_LIMIT; i++) {
    layout[i] = _TENOR_LAYOUT_ANY;
  }
}

/**
 * @brief Initialize the tensors ranks
 */
void
gst_tensors_rank_init (unsigned int ranks[])
{
  int i;
  for (i = 0; i < NNS_TENSOR_SIZE_LIMIT + NNS_TENSOR_SIZE_EXTRA_LIMIT; ++i) {
    ranks[i] = 0;
  }
}

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
 * @brief Check the tensor info is valid
 * @param info tensor info structure
 * @return TRUE if info is valid
 */
gboolean
gst_tensor_info_validate (const GstTensorInfo * info)
{
  g_return_val_if_fail (info != NULL, FALSE);

  if (info->type == _TENOR_END) {
    g_print
        ("Failed to validate tensor info. Please specify tensor type. e.g., type=uint8 ");
    GST_ERROR (
        "Failed to validate tensor info. Please specify tensor type. e.g., type=uint8 ");
    return FALSE;
  }

  /* validate tensor dimension */
  return gst_tensor_dimension_is_valid (info->dimension);
}

/**
 * @brief Initialize the tensors info structure
 * @param info tensors info structure to be initialized
 */
void
gst_tensors_info_init (GstTensorsInfo * info)
{
  guint i;

  g_return_if_fail (info != NULL);

  info->num_tensors = 0;
  info->extra = NULL;

  for (i = 0; i < NNS_TENSOR_SIZE_LIMIT; i++) {
    gst_tensor_info_init (&info->info[i]);
  }
}

/**
 * @brief Free allocated data in tensors info structure
 * @param info tensors info structure
 */
void
gst_tensors_info_free (GstTensorsInfo * info)
{
  guint i;

  g_return_if_fail (info != NULL);

  for (i = 0; i < NNS_TENSOR_SIZE_LIMIT; i++) {
    gst_tensor_info_free (&info->info[i]);
  }

  if (info->extra)
    gst_tensors_info_extra_free (info);
}

/**
 * @brief Check the tensors info is valid
 * @param info tensors info structure
 * @return TRUE if info is valid
 */
gboolean
gst_tensors_info_validate (const GstTensorsInfo * info)
{
  guint i;
  GstTensorInfo *_info;

  g_return_val_if_fail (info != NULL, FALSE);

  if (info->num_tensors < 1) {
    g_print
        ("Failed to validate tensors info. the number of tensors: %d. the number of tensors should be greater than 0.",
        info->num_tensors);
    GST_ERROR (
        "Failed to validate tensors info. the number of tensors: %d. the number of tensors should be greater than 0.",
        info->num_tensors);
    return FALSE;
  }

  for (i = 0; i < info->num_tensors; i++) {
    _info = gst_tensors_info_get_nth_info ((GstTensorsInfo *) info, i);

    if (!gst_tensor_info_validate (_info))
      return FALSE;
  }

  return TRUE;
}

/**
 * @brief Compare the tensor dimension.
 * @return TRUE if given tensors have same dimension.
 */
gboolean
gst_tensor_dimension_is_equal (const tensor_dim dim1, const tensor_dim dim2)
{
  guint i;

  /* Do not compare invalid dimensions. */
  if (!gst_tensor_dimension_is_valid (dim1) ||
      !gst_tensor_dimension_is_valid (dim2))
    return FALSE;

  for (i = 0; i < NNS_TENSOR_RANK_LIMIT; i++) {
    if (dim1[i] != dim2[i]) {
      /* Supposed dimension is same if remained dimension is 1. */
      if (dim1[i] > 1 || dim2[i] > 1)
        return FALSE;
    }
  }

  return TRUE;
}

/**
 * @brief Compare tensor info
 * @return TRUE if equal, FALSE if given tensor infos are invalid or not equal.
 */
gboolean
gst_tensor_info_is_equal (const GstTensorInfo * i1, const GstTensorInfo * i2)
{
  if (!gst_tensor_info_validate (i1) || !gst_tensor_info_validate (i2)) {
    return FALSE;
  }

  if (i1->type != i2->type) {
    g_print ("Tensor info is not equal. Given tensor types %d vs %d",
         i1->type, i2->type);
    return FALSE;
  }

  if (!gst_tensor_dimension_is_equal (i1->dimension, i2->dimension)) {
    g_print ("Tensor info is not equal. tensor dimensions is different.");
    return FALSE;
  }

  /* matched all */
  return TRUE;
}

/**
 * @brief Compare tensors info
 * @return TRUE if equal, FALSE if given tensor infos are invalid or not equal.
 */
gboolean
gst_tensors_info_is_equal (const GstTensorsInfo * i1, const GstTensorsInfo * i2)
{
  guint i;
  GstTensorInfo *_info1, *_info2;

  g_return_val_if_fail (i1 != NULL, FALSE);
  g_return_val_if_fail (i2 != NULL, FALSE);

  if (!gst_tensors_info_validate (i1) || !gst_tensors_info_validate (i2)) {
    return FALSE;
  }

  if (i1->num_tensors != i2->num_tensors) {
    g_print ("Tensors info is not equal. the number of tensors: %d vs %d. ",
        i1->num_tensors, i2->num_tensors);
    return FALSE;
  }

  for (i = 0; i < i1->num_tensors; i++) {
    _info1 = gst_tensors_info_get_nth_info ((GstTensorsInfo *) i1, i);
    _info2 = gst_tensors_info_get_nth_info ((GstTensorsInfo *) i2, i);

    if (!gst_tensor_info_is_equal (_info1, _info2)) {
      return FALSE;
    }
  }

  /* matched all */
  return TRUE;
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
  GstTensorInfo *temp;
  guint i;

  g_return_val_if_fail (info != NULL, FALSE);

  if (info->extra) {
    return TRUE;
  }

  temp = g_try_new0 (GstTensorInfo, NNS_TENSOR_SIZE_EXTRA_LIMIT);
  if (!temp) {
    g_print ("Failed to allocate memory for extra tensors info");
    return FALSE;
  }

  for (i = 0; i < NNS_TENSOR_SIZE_EXTRA_LIMIT; ++i) {
    gst_tensor_info_init (&temp[i]);
  }

  info->extra = temp;

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
      g_print ("Invalid param, types (%d) max (%d)\n",
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
 * @brief Get the rank of tensor dimension.
 * @param dim tensor dimension.
 * @return tensor rank (Minimum rank is 1 if given dimension is valid)
 */
guint
gst_tensor_dimension_get_rank (const tensor_dim dim)
{
  guint i;

  for (i = 0; i < NNS_TENSOR_RANK_LIMIT; i++) {
    if (dim[i] == 0)
      break;
  }

  return i;
}

/**
 * @brief Check the tensor dimension is valid
 * @param dim tensor dimension
 * @return TRUE if dimension is valid
 */
gboolean
gst_tensor_dimension_is_valid (const tensor_dim dim)
{
  guint i;
  gboolean is_valid = FALSE;

  i = gst_tensor_dimension_get_rank (dim);
  if (i == 0)
    goto done;

  for (; i < NNS_TENSOR_RANK_LIMIT; i++) {
    if (dim[i] > 0)
      goto done;
  }

  is_valid = TRUE;

done:
  if (!is_valid) {
    g_print
        ("Failed to validate tensor dimension. The dimension string should be in the form of d1:...:d8, d1:d2:d3:d4, d1:d2:d3, d1:d2, or d1. Here, dN is a positive integer.");
    GST_ERROR (
        "Failed to validate tensor dimension. The dimension string should be in the form of d1:...:d8, d1:d2:d3:d4, d1:d2:d3, d1:d2, or d1. Here, dN is a positive integer.");
  }

  return is_valid;
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