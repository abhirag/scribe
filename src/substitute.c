#include "substitute.h"

#include <janet.h>
#include <md4c.h>
#include <stdbool.h>
#include <stdlib.h>

#include "lisp.h"
#include "query.h"
#include "trace.h"

INIT_TRACE;

static int render_verbatim(MD_CHAR* text, md_substitute_data* data);
static int render_verbatim_sds(sds text, md_substitute_data* data);
static int render_verbatim_len(MD_CHAR* text, MD_SIZE size,
                               md_substitute_data* data);
static int render_h_block(MD_BLOCK_H_DETAIL* detail, md_substitute_data* data);
static void render_open_ol_block(MD_BLOCK_OL_DETAIL* detail,
                                 md_substitute_data* data);
static void render_closed_ol_block(MD_BLOCK_OL_DETAIL* detail,
                                   md_substitute_data* data);
static int render_li_block(MD_BLOCK_LI_DETAIL* detail,
                           md_substitute_data* data);
static int render_open_a_span(MD_SPAN_A_DETAIL* detail,
                              md_substitute_data* data);
static int render_closed_a_span(MD_SPAN_A_DETAIL* detail,
                                md_substitute_data* data);
static int render_open_img_span(MD_SPAN_IMG_DETAIL* detail,
                                md_substitute_data* data);
static int render_closed_img_span(MD_SPAN_IMG_DETAIL* detail,
                                  md_substitute_data* data);
static int render_block_quote(md_substitute_data* data);
static sds code_block_lang(MD_BLOCK_CODE_DETAIL* detail);
static int accumulate_code_text(MD_CHAR* text, MD_SIZE size,
                                md_substitute_data* data);
static int process_scribe_code_block(MD_BLOCK_CODE_DETAIL* detail,
                                     md_substitute_data* data);
static int process_code_block(MD_BLOCK_CODE_DETAIL* detail,
                              md_substitute_data* data);
static int enter_block_callback(MD_BLOCKTYPE type, void* detail,
                                void* userdata);
static int leave_block_callback(MD_BLOCKTYPE type, void* detail,
                                void* userdata);
static int enter_span_callback(MD_SPANTYPE type, void* detail, void* userdata);
static int leave_span_callback(MD_SPANTYPE type, void* detail, void* userdata);
static int text_callback(MD_TEXTTYPE type, const MD_CHAR* text, MD_SIZE size,
                         void* userdata);
static void debug_log_callback(const char* msg, void* userdata);

static int render_verbatim(MD_CHAR* text, md_substitute_data* data) {
  START_ZONE;
  sds str = sdscat(data->output, text);
  if (!str) {
    message_fatal(
        "substitute::render_verbatim failed in appending to SDS string\n");
    END_ZONE;
    return -1;
  }
  data->output = str;
  END_ZONE;
  return 0;
}

static int render_verbatim_sds(sds text, md_substitute_data* data) {
  START_ZONE;
  sds str = sdscatsds(data->output, text);
  if (!str) {
    message_fatal(
        "substitute::render_verbatim_sds failed in appending to SDS string\n");
    END_ZONE;
    return -1;
  }
  data->output = str;
  END_ZONE;
  return 0;
}

static int render_verbatim_len(MD_CHAR* text, MD_SIZE size,
                               md_substitute_data* data) {
  START_ZONE;
  sds str = sdscatlen(data->output, text, size);
  if (!str) {
    message_fatal(
        "substitute::render_verbatim_len failed in appending to SDS string\n");
    END_ZONE;
    return -1;
  }
  data->output = str;
  END_ZONE;
  return 0;
}

static int render_h_block(MD_BLOCK_H_DETAIL* detail, md_substitute_data* data) {
  START_ZONE;
  sds to_render = sdsempty();
  for (size_t i = 0; i < detail->level; i += 1) {
    to_render = sdscat(to_render, "#");
  }
  to_render = sdscat(to_render, " ");
  if (!to_render) {
    message_fatal(
        "substitute::render_h_block failed in appending to SDS string\n");
    sdsfree(to_render);
    END_ZONE;
    return -1;
  }
  int rc = render_verbatim_sds(to_render, data);
  sdsfree(to_render);
  END_ZONE;
  return rc;
}

static void render_open_ol_block(MD_BLOCK_OL_DETAIL* detail,
                                 md_substitute_data* data) {
  START_ZONE;
  data->is_ordered_list = true;
  data->current_index = detail->start;
  END_ZONE;
}

static void render_closed_ol_block(MD_BLOCK_OL_DETAIL* detail,
                                   md_substitute_data* data) {
  START_ZONE;
  data->is_ordered_list = false;
  data->current_index = 0;
  END_ZONE;
}

static int render_li_block(MD_BLOCK_LI_DETAIL* detail,
                           md_substitute_data* data) {
  START_ZONE;
  int rc;
  sds to_render = sdsempty();
  if (data->is_ordered_list) {
    char current_index_str[1024];
    snprintf(current_index_str, 1024, "%u", data->current_index);
    to_render = sdscatfmt(to_render, "%s. ", current_index_str);
    if (!to_render) {
      message_fatal(
          "substitute::render_li_block failed in appending to SDS string\n");
      goto error_end;
    }
    rc = render_verbatim_sds(to_render, data);
    data->current_index += 1;
  } else {
    rc = render_verbatim("- ", data);
  }
  sdsfree(to_render);
  END_ZONE;
  return rc;
error_end:
  sdsfree(to_render);
  END_ZONE;
  return -1;
}

static int render_open_a_span(MD_SPAN_A_DETAIL* detail,
                              md_substitute_data* data) {
  START_ZONE;
  int rc = render_verbatim("[", data);
  END_ZONE;
  return rc;
}

static int render_closed_a_span(MD_SPAN_A_DETAIL* detail,
                                md_substitute_data* data) {
  START_ZONE;
  int rc = render_verbatim("](", data);
  if (rc == -1) {
    goto error_end;
  }
  rc = render_verbatim_len(detail->href.text, detail->href.size, data);
  if (rc == -1) {
    goto error_end;
  }
  if (detail->title.text) {
    rc = render_verbatim(" \"", data);
    if (rc == -1) {
      goto error_end;
    }
    rc = render_verbatim_len(detail->title.text, detail->title.size, data);
    if (rc == -1) {
      goto error_end;
    }
    rc = render_verbatim("\"", data);
    if (rc == -1) {
      goto error_end;
    }
  }
  rc = render_verbatim(")", data);
  END_ZONE;
  return rc;
error_end:
  END_ZONE;
  return -1;
}

static int render_open_img_span(MD_SPAN_IMG_DETAIL* detail,
                                md_substitute_data* data) {
  START_ZONE;
  int rc = render_verbatim("![", data);
  END_ZONE;
  return rc;
}

static int render_closed_img_span(MD_SPAN_IMG_DETAIL* detail,
                                  md_substitute_data* data) {
  START_ZONE;
  int rc = render_verbatim("](", data);
  if (rc == -1) {
    goto error_end;
  }
  rc = render_verbatim_len(detail->src.text, detail->src.size, data);
  if (rc == -1) {
    goto error_end;
  }
  if (detail->title.text) {
    rc = render_verbatim(" \"", data);
    if (rc == -1) {
      goto error_end;
    }
    rc = render_verbatim_len(detail->title.text, detail->title.size, data);
    if (rc == -1) {
      goto error_end;
    }
    rc = render_verbatim("\"", data);
    if (rc == -1) {
      goto error_end;
    }
  }
  rc = render_verbatim(")", data);
  END_ZONE;
  return rc;
error_end:
  END_ZONE;
  return -1;
}

static int render_block_quote(md_substitute_data* data) {
  START_ZONE;
  int rc = render_verbatim(">", data);
  END_ZONE;
  return rc;
}

static sds code_block_lang(MD_BLOCK_CODE_DETAIL* detail) {
  START_ZONE;
  size_t initlen = (size_t)(detail->lang.size);
  sds lang = sdsnewlen(detail->lang.text, initlen);
  END_ZONE;
  return lang;
}

static int accumulate_code_text(MD_CHAR* text, MD_SIZE size,
                                md_substitute_data* data) {
  START_ZONE;
  sds str = sdscatlen(data->code_text, text, size);
  if (!str) {
    message_fatal(
        "substitute::accumulate_code_text failed in appending to SDS string\n");
    END_ZONE;
    return -1;
  }
  data->code_text = str;
  END_ZONE;
  return 0;
}

static int process_scribe_code_block(MD_BLOCK_CODE_DETAIL* detail,
                                     md_substitute_data* data) {
  START_ZONE;
  sds lang = get_language();
  JanetTable* env = lisp_init_env();
  register_modules(env);
  Janet out = {0};
  int rc = lisp_execute_script(env, data->code_text, &out);
  if (rc != 0) {
    message_fatal(
        "substitute::process_scribe_code_block failed in code execution");
    goto end;
  }
  JanetString jstr = janet_getstring(&out, 0);
  rc = render_verbatim("```", data);
  if (rc == -1) {
    goto end;
  }
  rc = render_verbatim_sds(lang, data);
  if (rc == -1) {
    goto end;
  }
  rc = render_verbatim("\n", data);
  if (rc == -1) {
    goto end;
  }
  rc = render_verbatim(jstr, data);
  if (rc == -1) {
    goto end;
  }
  rc = render_verbatim("\n```", data);
  if (rc == -1) {
    goto end;
  }
end:
  sdsfree(lang);
  lisp_terminate();
  END_ZONE;
  return rc;
}

static int process_code_block(MD_BLOCK_CODE_DETAIL* detail,
                              md_substitute_data* data) {
  START_ZONE;
  int rc = 0;
  sds lang = code_block_lang(detail);
  sds scribe_lang = sdsnew("scribe");
  if (sdscmp(scribe_lang, lang) == 0) {
    rc = process_scribe_code_block(detail, data);
    goto end;
  }
  rc = render_verbatim("```", data);
  if (rc == -1) {
    goto end;
  }
  rc = render_verbatim_sds(lang, data);
  if (rc == -1) {
    goto end;
  }
  rc = render_verbatim("\n", data);
  if (rc == -1) {
    goto end;
  }
  rc = render_verbatim_sds(data->code_text, data);
  if (rc == -1) {
    goto end;
  }
  rc = render_verbatim("```", data);
  if (rc == -1) {
    goto end;
  }
end:
  sdsfree(lang);
  sdsfree(scribe_lang);
  END_ZONE;
  return rc;
}

static int enter_block_callback(MD_BLOCKTYPE type, void* detail,
                                void* userdata) {
  md_substitute_data* d = (md_substitute_data*)userdata;
  switch (type) {
    case MD_BLOCK_HTML:  // noop
      break;
    case MD_BLOCK_DOC:  // noop
      break;
    case MD_BLOCK_QUOTE:
      if (render_block_quote(d) == -1) {
        return -1;
      }
      break;
    case MD_BLOCK_UL:  // noop
      break;
    case MD_BLOCK_OL:
      render_open_ol_block((MD_BLOCK_OL_DETAIL*)detail, d);
      break;
    case MD_BLOCK_LI:
      if (render_li_block((MD_BLOCK_LI_DETAIL*)detail, d) == -1) {
        return -1;
      }
      break;
    case MD_BLOCK_HR:  // noop
      break;
    case MD_BLOCK_H:
      if (render_h_block((MD_BLOCK_H_DETAIL*)detail, d) == -1) {
        return -1;
      }
      break;
    case MD_BLOCK_CODE:  // noop
      break;
    case MD_BLOCK_P:  // noop
      break;
    case MD_BLOCK_TABLE:  // noop
      break;
    case MD_BLOCK_THEAD:  // noop
      break;
    case MD_BLOCK_TBODY:  // noop
      break;
    case MD_BLOCK_TR:  // noop
      break;
    case MD_BLOCK_TH:  // noop
      break;
    case MD_BLOCK_TD:  // noop
      break;
  }
  return 0;
}

static int leave_block_callback(MD_BLOCKTYPE type, void* detail,
                                void* userdata) {
  md_substitute_data* d = (md_substitute_data*)userdata;
  switch (type) {
    case MD_BLOCK_HTML:
      if (render_verbatim("\n", d) == -1) {
        return -1;
      }
      break;
    case MD_BLOCK_DOC:  // noop
      break;
    case MD_BLOCK_QUOTE:
      if (render_verbatim("\n", d) == -1) {
        return -1;
      }
      break;
    case MD_BLOCK_UL:
      if (render_verbatim("\n", d) == -1) {
        return -1;
      }
      break;
    case MD_BLOCK_OL:
      render_closed_ol_block((MD_BLOCK_OL_DETAIL*)detail, d);
      break;
    case MD_BLOCK_LI:
      if (render_verbatim("\n", d) == -1) {
        return -1;
      }
      break;
    case MD_BLOCK_HR:  // noop
      break;
    case MD_BLOCK_H:
      if (render_verbatim("\n\n", d) == -1) {
        return -1;
      }
      break;
    case MD_BLOCK_CODE:
      if (process_code_block((MD_BLOCK_CODE_DETAIL*)detail, d) == -1) {
        return -1;
      }
      break;
    case MD_BLOCK_P:
      if (render_verbatim("\n\n", d) == -1) {
        return -1;
      }
      break;
    case MD_BLOCK_TABLE:  // noop
      break;
    case MD_BLOCK_THEAD:  // noop
      break;
    case MD_BLOCK_TBODY:  // noop
      break;
    case MD_BLOCK_TR:  // noop
      break;
    case MD_BLOCK_TH:  // noop
      break;
    case MD_BLOCK_TD:  // noop
      break;
  }
  return 0;
}

static int enter_span_callback(MD_SPANTYPE type, void* detail, void* userdata) {
  md_substitute_data* d = (md_substitute_data*)userdata;
  switch (type) {
    case MD_SPAN_EM:
      if (render_verbatim("*", d) == -1) {
        return -1;
      }
      break;
    case MD_SPAN_STRONG:
      if (render_verbatim("**", d) == -1) {
        return -1;
      }
      break;
    case MD_SPAN_A:
      if (render_open_a_span((MD_SPAN_A_DETAIL*)detail, d) == -1) {
        return -1;
      }
      break;
    case MD_SPAN_IMG:
      if (render_open_img_span((MD_SPAN_IMG_DETAIL*)detail, d) == -1) {
        return -1;
      }
      break;
    case MD_SPAN_CODE:
      break;
    case MD_SPAN_DEL:  // noop
      break;
    case MD_SPAN_LATEXMATH:  // noop
      break;
    case MD_SPAN_LATEXMATH_DISPLAY:  // noop
      break;
    case MD_SPAN_WIKILINK:  // noop
      break;
    case MD_SPAN_U:  // noop
      break;
  }
  return 0;
}

static int leave_span_callback(MD_SPANTYPE type, void* detail, void* userdata) {
  md_substitute_data* d = (md_substitute_data*)userdata;
  switch (type) {
    case MD_SPAN_EM:
      if (render_verbatim("*", d) == -1) {
        return -1;
      }
      break;
    case MD_SPAN_STRONG:
      if (render_verbatim("**", d) == -1) {
        return -1;
      }
      break;
    case MD_SPAN_A:
      if (render_closed_a_span((MD_SPAN_A_DETAIL*)detail, d) == -1) {
        return -1;
      }
      break;
    case MD_SPAN_IMG:
      if (render_closed_img_span((MD_SPAN_IMG_DETAIL*)detail, d) == -1) {
        return -1;
      }
      break;
    case MD_SPAN_CODE:
      break;
    case MD_SPAN_DEL:  // noop
      break;
    case MD_SPAN_LATEXMATH:  // noop
      break;
    case MD_SPAN_LATEXMATH_DISPLAY:  // noop
      break;
    case MD_SPAN_WIKILINK:  // noop
      break;
    case MD_SPAN_U:  // noop
      break;
  }
  return 0;
}

static int text_callback(MD_TEXTTYPE type, const MD_CHAR* text, MD_SIZE size,
                         void* userdata) {
  md_substitute_data* d = (md_substitute_data*)userdata;
  switch (type) {
    case MD_TEXT_CODE:
      if (accumulate_code_text(text, size, d) == -1) {
        return -1;
      }
      break;
    case MD_TEXT_BR:
      if (render_verbatim("  \n", d) == -1) {
        return -1;
      }
      break;
    default:
      if (render_verbatim_len(text, size, d) == -1) {
        return -1;
      }
      break;
  }
  return 0;
}

static void debug_log_callback(const char* msg, void* userdata) {}

int md_substitute(const MD_CHAR* input, MD_SIZE input_size,
                  md_substitute_data* data) {
  MD_PARSER parser = {0,
                      0,
                      enter_block_callback,
                      leave_block_callback,
                      enter_span_callback,
                      leave_span_callback,
                      text_callback,
                      debug_log_callback,
                      (void*)0};

  return md_parse(input, input_size, &parser, data);
}

#ifdef UNIT_TEST_SUBSTITUTE

#include <string.h>

#include "test_deps/utest.h"

UTEST(substitute, sample_test) {
  md_substitute_data d = {.code_text = sdsempty(),
                          .output = sdsempty(),
                          .is_ordered_list = false,
                          .current_index = 0};
  char const* input =
      "this is a code block.\n\n```scribe\n(core/list-paths)\n```";
  md_substitute(input, strlen(input), &d);
  log_trace("input --\n%s", input);
  log_trace("output --\n%s", d.output);
  sdsfree(d.code_text);
  sdsfree(d.output);
  ASSERT_TRUE(true);
}

UTEST_MAIN();

#endif
