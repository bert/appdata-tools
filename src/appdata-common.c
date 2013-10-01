/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2013 Richard Hughes <richard@hughsie.com>
 *
 * Licensed under the GNU General Public License Version 2
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <string.h>

#include "appdata-common.h"
#include "appdata-problem.h"

typedef enum {
	APPDATA_SECTION_UNKNOWN,
	APPDATA_SECTION_APPLICATION,
	APPDATA_SECTION_DESCRIPTION,
	APPDATA_SECTION_DESCRIPTION_PARA,
	APPDATA_SECTION_DESCRIPTION_UL,
	APPDATA_SECTION_DESCRIPTION_UL_LI,
	APPDATA_SECTION_ID,
	APPDATA_SECTION_LICENCE,
	APPDATA_SECTION_NAME,
	APPDATA_SECTION_SCREENSHOT,
	APPDATA_SECTION_SCREENSHOTS,
	APPDATA_SECTION_SUMMARY,
	APPDATA_SECTION_UPDATECONTACT,
	APPDATA_SECTION_PROJECT_GROUP,
	APPDATA_SECTION_URL,
	APPDATA_SECTION_LAST
} AppdataSection;

/**
 * appdata_selection_from_string:
 */
static AppdataSection
appdata_selection_from_string (const gchar *element_name)
{
	if (g_strcmp0 (element_name, "application") == 0)
		return APPDATA_SECTION_APPLICATION;
	if (g_strcmp0 (element_name, "id") == 0)
		return APPDATA_SECTION_ID;
	if (g_strcmp0 (element_name, "licence") == 0)
		return APPDATA_SECTION_LICENCE;
	if (g_strcmp0 (element_name, "screenshots") == 0)
		return APPDATA_SECTION_SCREENSHOTS;
	if (g_strcmp0 (element_name, "screenshot") == 0)
		return APPDATA_SECTION_SCREENSHOT;
	if (g_strcmp0 (element_name, "name") == 0)
		return APPDATA_SECTION_NAME;
	if (g_strcmp0 (element_name, "summary") == 0)
		return APPDATA_SECTION_SUMMARY;
	if (g_strcmp0 (element_name, "url") == 0)
		return APPDATA_SECTION_URL;
	if (g_strcmp0 (element_name, "description") == 0)
		return APPDATA_SECTION_DESCRIPTION;
	if (g_strcmp0 (element_name, "p") == 0)
		return APPDATA_SECTION_DESCRIPTION_PARA;
	if (g_strcmp0 (element_name, "ul") == 0)
		return APPDATA_SECTION_DESCRIPTION_UL;
	if (g_strcmp0 (element_name, "li") == 0)
		return APPDATA_SECTION_DESCRIPTION_UL_LI;
	if (g_strcmp0 (element_name, "updatecontact") == 0)
		return APPDATA_SECTION_UPDATECONTACT;
	if (g_strcmp0 (element_name, "project_group") == 0)
		return APPDATA_SECTION_PROJECT_GROUP;
	return APPDATA_SECTION_UNKNOWN;
}

/**
 * appdata_selection_to_string:
 */
static const gchar *
appdata_selection_to_string (AppdataSection section)
{
	if (section == APPDATA_SECTION_APPLICATION)
		return "application";
	if (section == APPDATA_SECTION_ID)
		return "id";
	if (section == APPDATA_SECTION_LICENCE)
		return "licence";
	if (section == APPDATA_SECTION_SCREENSHOTS)
		return "screenshots";
	if (section == APPDATA_SECTION_SCREENSHOT)
		return "screenshot";
	if (section == APPDATA_SECTION_NAME)
		return "name";
	if (section == APPDATA_SECTION_SUMMARY)
		return "summary";
	if (section == APPDATA_SECTION_URL)
		return "url";
	if (section == APPDATA_SECTION_DESCRIPTION)
		return "description";
	if (section == APPDATA_SECTION_DESCRIPTION_PARA)
		return "p";
	if (section == APPDATA_SECTION_DESCRIPTION_UL)
		return "ul";
	if (section == APPDATA_SECTION_DESCRIPTION_UL_LI)
		return "li";
	if (section == APPDATA_SECTION_UPDATECONTACT)
		return "updatecontact";
	if (section == APPDATA_SECTION_PROJECT_GROUP)
		return "project_group";
	return NULL;
}

/**
 * appdata_add_problem:
 */
static void
appdata_add_problem (GList **problems, AppdataProblemKind kind, const gchar *str)
{
	GList *l;
	AppdataProblem *problem;

	/* find if it's already been added */
	for (l = *problems; l != NULL; l = l->next) {
		problem = l->data;
		if (g_strcmp0 (str, problem->description) == 0)
			return;
	}

	/* add new problem to list */
	problem = appdata_problem_new ();
	problem->kind = kind;
	problem->description = g_strdup (str);
	*problems = g_list_prepend (*problems, problem);
}

typedef enum {
	APPDATA_KIND_UNKNOWN,
	APPDATA_KIND_DESKTOP,
	APPDATA_KIND_FONT,
	APPDATA_KIND_INPUTMETHOD,
	APPDATA_KIND_LAST
} AppdataKind;

typedef struct {
	AppdataSection	 section;
	gchar		*id;
	AppdataKind	 kind;
	gchar		*name;
	gchar		*summary;
	gchar		*licence;
	gchar		*updatecontact;
	gchar		*project_group;
	gchar		*url;
	GList		**problems;
	guint		 number_paragraphs;
	gboolean	 tag_translated;
	gboolean	 previous_para_was_short;
	gboolean	 seen_application;
	guint		 translations_name;
	guint		 translations_summary;
	guint		 translations_description;
	GKeyFile	*config;
	GPtrArray	*screenshots;
	gboolean	 has_default_screenshot;
} AppdataHelper;

/**
 * appdata_id_type_from_string:
 */
static gboolean
appdata_id_type_from_string (const gchar *id_type)
{
	if (g_strcmp0 (id_type, "desktop") == 0)
		return APPDATA_KIND_DESKTOP;
	if (g_strcmp0 (id_type, "font") == 0)
		return APPDATA_KIND_FONT;
	if (g_strcmp0 (id_type, "inputmethod") == 0)
		return APPDATA_KIND_INPUTMETHOD;
	return APPDATA_KIND_UNKNOWN;
}

/**
 * appdata_start_element_fn:
 */
static void
appdata_start_element_fn (GMarkupParseContext *context,
			  const gchar *element_name,
			  const gchar **attribute_names,
			  const gchar **attribute_values,
			  gpointer user_data,
			  GError **error)
{
	AppdataHelper *helper = (AppdataHelper *) user_data;
	AppdataSection new;
	const gchar *tmp;
	guint i;

	new = appdata_selection_from_string (element_name);

	/* all tags are untranslated until found otherwise */
	helper->tag_translated = FALSE;
	for (i = 0; attribute_names[i] != NULL; i++) {
		if (g_strcmp0 (attribute_names[i], "xml:lang") == 0) {
			tmp = attribute_values[i];
			if (g_strcmp0 (tmp, "C") == 0) {
				appdata_add_problem (helper->problems,
						     APPDATA_PROBLEM_KIND_ATTRIBUTE_INVALID,
						     "xml:lang should never be 'C'");
			}
			helper->tag_translated = TRUE;
			if (new == APPDATA_SECTION_NAME)
				helper->translations_name++;
			else if (new == APPDATA_SECTION_SUMMARY)
				helper->translations_summary++;
			else if (new == APPDATA_SECTION_DESCRIPTION_PARA)
				helper->translations_description++;
			break;
		}
	}

	/* unknown -> application */
	if (helper->section == APPDATA_SECTION_UNKNOWN) {
		if (new == APPDATA_SECTION_APPLICATION) {
			/* valid */
			helper->section = new;
			if (helper->seen_application) {
				appdata_add_problem (helper->problems,
						     APPDATA_PROBLEM_KIND_MARKUP_INVALID,
						     "<application> used more than once");
			}
			helper->seen_application = TRUE;
			return;
		}
		g_set_error (error, 1, 0,
			     "start tag <%s> not allowed from section <%s>",
			     element_name,
			     appdata_selection_to_string (helper->section));
		return;
	}

	/* application -> various */
	if (helper->section == APPDATA_SECTION_APPLICATION) {
		switch (new) {
		case APPDATA_SECTION_ID:
			tmp = NULL;
			for (i = 0; attribute_names[i] != NULL; i++) {
				if (g_strcmp0 (attribute_names[i], "type") == 0) {
					tmp = attribute_values[i];
					break;
				}
			}
			if (tmp == NULL) {
				helper->kind = APPDATA_KIND_DESKTOP;
				appdata_add_problem (helper->problems,
						     APPDATA_PROBLEM_KIND_ATTRIBUTE_MISSING,
						     "no type attribute in <id>");
			} else {
				helper->kind = appdata_id_type_from_string (tmp);
				if (helper->kind == APPDATA_KIND_UNKNOWN) {
					appdata_add_problem (helper->problems,
							     APPDATA_PROBLEM_KIND_ATTRIBUTE_INVALID,
							     "<id> has invalid type attribute");
				}
			}
			helper->section = new;
			break;
		case APPDATA_SECTION_URL:
			tmp = NULL;
			for (i = 0; attribute_names[i] != NULL; i++) {
				if (g_strcmp0 (attribute_names[i], "type") == 0) {
					tmp = attribute_values[i];
					break;
				}
			}
			if (tmp == NULL) {
				appdata_add_problem (helper->problems,
						     APPDATA_PROBLEM_KIND_ATTRIBUTE_MISSING,
						     "no type attribute in <url>");
			}
			if (g_strcmp0 (tmp, "homepage") != 0) {
				appdata_add_problem (helper->problems,
						     APPDATA_PROBLEM_KIND_ATTRIBUTE_INVALID,
						     "<url> has invalid type attribute");
			}
			helper->section = new;
			break;
		case APPDATA_SECTION_NAME:
		case APPDATA_SECTION_SUMMARY:
		case APPDATA_SECTION_LICENCE:
		case APPDATA_SECTION_DESCRIPTION:
		case APPDATA_SECTION_SCREENSHOTS:
		case APPDATA_SECTION_UPDATECONTACT:
		case APPDATA_SECTION_PROJECT_GROUP:
			/* valid */
			helper->section = new;
			break;
		default:
			g_set_error (error, 1, 0,
				     "start tag <%s> not allowed from section <%s>",
				     element_name,
				     appdata_selection_to_string (helper->section));
		}
		return;
	}

	/* description -> p or -> ul */
	if (helper->section == APPDATA_SECTION_DESCRIPTION) {
		switch (new) {
		case APPDATA_SECTION_DESCRIPTION_PARA:
			helper->section = new;

			/* previous paragraph wasn't long enough */
			if (helper->previous_para_was_short) {
				appdata_add_problem (helper->problems,
						     APPDATA_PROBLEM_KIND_STYLE_INCORRECT,
						     "<p> is too short [p]");
			}
			helper->previous_para_was_short = FALSE;
			break;
		case APPDATA_SECTION_DESCRIPTION_UL:
			/* ul without a leading para */
			if (helper->number_paragraphs < 1) {
				appdata_add_problem (helper->problems,
						     APPDATA_PROBLEM_KIND_STYLE_INCORRECT,
						     "<ul> cannot start a description");
			}
			/* we allow the previous paragraph to be short to
			 * introduce the list */
			helper->previous_para_was_short = FALSE;
			helper->section = new;
			break;
		default:
			g_set_error (error, 1, 0,
				     "start tag <%s> not allowed from section <%s>",
				     element_name,
				     appdata_selection_to_string (helper->section));
		}
		return;
	}

	/* ul -> li */
	if (helper->section == APPDATA_SECTION_DESCRIPTION_UL) {
		switch (new) {
		case APPDATA_SECTION_DESCRIPTION_UL_LI:
			/* valid */
			helper->section = new;
			break;
		default:
			g_set_error (error, 1, 0,
				     "start tag <%s> not allowed from section <%s>",
				     element_name,
				     appdata_selection_to_string (helper->section));
		}
		return;
	}

	/* unknown -> application */
	if (helper->section == APPDATA_SECTION_SCREENSHOTS) {
		if (new == APPDATA_SECTION_SCREENSHOT) {
			tmp = NULL;
			for (i = 0; attribute_names[i] != NULL; i++) {
				if (g_strcmp0 (attribute_names[i], "type") == 0) {
					tmp = attribute_values[i];
					break;
				}
			}
			if (tmp != NULL && g_strcmp0 (tmp, "default") != 0) {
				appdata_add_problem (helper->problems,
						     APPDATA_PROBLEM_KIND_ATTRIBUTE_INVALID,
						     "<screenshot> has unknown type");
			}
			if (g_strcmp0 (tmp, "default") == 0) {
				if (helper->has_default_screenshot) {
					appdata_add_problem (helper->problems,
							     APPDATA_PROBLEM_KIND_MARKUP_INVALID,
							     "<screenshot> has more than one default");
				}
				helper->has_default_screenshot = TRUE;
			}
			helper->section = new;
			return;
		}
		g_set_error (error, 1, 0,
			     "start tag <%s> not allowed from section <%s>",
			     element_name,
			     appdata_selection_to_string (helper->section));
		return;
	}

	g_set_error (error, 1, 0,
		     "start tag <%s> not allowed from section <%s>",
		     element_name,
		     appdata_selection_to_string (helper->section));
}

/**
 * appdata_end_element_fn:
 */
static void
appdata_end_element_fn (GMarkupParseContext *context,
			const gchar *element_name,
			gpointer user_data,
			GError **error)
{
	AppdataHelper *helper = (AppdataHelper *) user_data;
	AppdataSection new;

	new = appdata_selection_from_string (element_name);
	if (helper->section == APPDATA_SECTION_APPLICATION) {
		if (new == APPDATA_SECTION_APPLICATION) {
			/* valid */
			helper->section = APPDATA_SECTION_UNKNOWN;
			return;
		}
		g_set_error (error, 1, 0,
			     "end tag <%s> not allowed from section <%s>",
			     element_name,
			     appdata_selection_to_string (helper->section));
		return;
	}

	/* </id> */
	if (helper->section == APPDATA_SECTION_ID &&
	    new == APPDATA_SECTION_ID) {
		/* valid */
		helper->section = APPDATA_SECTION_APPLICATION;
		return;
	}

	/* </licence> */
	if (helper->section == APPDATA_SECTION_LICENCE &&
	    new == APPDATA_SECTION_LICENCE) {
		/* valid */
		helper->section = APPDATA_SECTION_APPLICATION;
		return;
	}

	/* </p> */
	if (helper->section == APPDATA_SECTION_DESCRIPTION_PARA &&
	    new == APPDATA_SECTION_DESCRIPTION_PARA) {
		/* valid */
		helper->section = APPDATA_SECTION_DESCRIPTION;
		return;
	}

	/* </li> */
	if (helper->section == APPDATA_SECTION_DESCRIPTION_UL_LI &&
	    new == APPDATA_SECTION_DESCRIPTION_UL_LI) {
		/* valid */
		helper->section = APPDATA_SECTION_DESCRIPTION_UL;
		return;
	}

	/* </ul> */
	if (helper->section == APPDATA_SECTION_DESCRIPTION_UL &&
	    new == APPDATA_SECTION_DESCRIPTION_UL) {
		/* valid */
		helper->section = APPDATA_SECTION_DESCRIPTION;
		return;
	}

	/* </description> */
	if (helper->section == APPDATA_SECTION_DESCRIPTION &&
	    new == APPDATA_SECTION_DESCRIPTION) {

		/* previous paragraph wasn't long enough */
		if (helper->previous_para_was_short) {
			appdata_add_problem (helper->problems,
					     APPDATA_PROBLEM_KIND_STYLE_INCORRECT,
					     "<p> is too short");
		}

		/* valid */
		helper->section = APPDATA_SECTION_APPLICATION;
		return;
	}

	/* </screenshot> */
	if (helper->section == APPDATA_SECTION_SCREENSHOT &&
	    new == APPDATA_SECTION_SCREENSHOT) {
		/* valid */
		helper->section = APPDATA_SECTION_SCREENSHOTS;
		return;
	}

	/* </screenshots> */
	if (helper->section == APPDATA_SECTION_SCREENSHOTS &&
	    new == APPDATA_SECTION_SCREENSHOTS) {
		/* valid */
		helper->section = APPDATA_SECTION_APPLICATION;
		return;
	}

	/* </url> */
	if (helper->section == APPDATA_SECTION_URL &&
	    new == APPDATA_SECTION_URL) {
		/* valid */
		helper->section = APPDATA_SECTION_APPLICATION;
		return;
	}

	/* </name> */
	if (helper->section == APPDATA_SECTION_NAME &&
	    new == APPDATA_SECTION_NAME) {
		/* valid */
		helper->section = APPDATA_SECTION_APPLICATION;
		return;
	}

	/* </summary> */
	if (helper->section == APPDATA_SECTION_SUMMARY &&
	    new == APPDATA_SECTION_SUMMARY) {
		/* valid */
		helper->section = APPDATA_SECTION_APPLICATION;
		return;
	}

	/* </updatecontact> */
	if (helper->section == APPDATA_SECTION_UPDATECONTACT &&
	    new == APPDATA_SECTION_UPDATECONTACT) {
		/* valid */
		helper->section = APPDATA_SECTION_APPLICATION;
		return;
	}

	/* </project_group> */
	if (helper->section == APPDATA_SECTION_PROJECT_GROUP &&
	    new == APPDATA_SECTION_PROJECT_GROUP) {
		/* valid */
		helper->section = APPDATA_SECTION_APPLICATION;
		return;
	}

	g_set_error (error, 1, 0,
		     "end tag <%s> not allowed from section <%s>",
		     element_name,
		     appdata_selection_to_string (helper->section));
}

/**
 * appdata_check_id_for_kind:
 */
static gboolean
appdata_check_id_for_kind (const gchar *id, AppdataKind kind)
{
	if (kind == APPDATA_KIND_DESKTOP)
		return g_str_has_suffix (id, ".desktop");
	if (kind == APPDATA_KIND_FONT)
		return g_str_has_suffix (id, ".ttf") ||
			g_str_has_suffix (id, ".odf");
	if (kind == APPDATA_KIND_INPUTMETHOD)
		return g_str_has_suffix (id, ".xml") ||
			g_str_has_suffix (id, ".db");
	return FALSE;
}

/**
 * appdata_has_fullstop_ending:
 *
 * Returns %TRUE if the string ends in a full stop, unless the string contains
 * multiple dots. This allows names such as "0 A.D." and summaries to end
 * with "..."
 */
static gboolean
appdata_has_fullstop_ending (const gchar *tmp)
{
	guint cnt = 0;
	guint i;
	guint str_len;

	for (i = 0; tmp[i] != '\0'; i++)
		if (tmp[i] == '.')
			cnt++;
	if (cnt++ > 1)
		return FALSE;
	str_len = strlen (tmp);
	if (str_len == 0)
		return FALSE;
	return tmp[str_len - 1] == '.';
}

/**
 * appdata_screenshot_already_exists:
 */
static gboolean
appdata_screenshot_already_exists (AppdataHelper *helper, const gchar *search)
{
	const gchar *tmp;
	guint i;

	for (i = 0; i < helper->screenshots->len; i++) {
		tmp = g_ptr_array_index (helper->screenshots, i);
		if (g_strcmp0 (tmp, search) == 0)
			return TRUE;
	}
	return FALSE;
}

/**
 * appdata_screenshot_check_remote_url:
 */
static gboolean
appdata_screenshot_check_remote_url (AppdataHelper *helper, const gchar *url)
{
	GError *error = NULL;
	gboolean got_network;
	gboolean ret = TRUE;
	gint exit_status = 0;
	const gchar *argv[] = { "/usr/bin/wget",
			        "--spider",
			        "--quiet",
			        url,
			        NULL };

	/* have we got network access */
	got_network = g_key_file_get_boolean (helper->config,
					      APPDATA_TOOLS_VALIDATE_GROUP_NAME,
					      "HasNetworkAccess", NULL);
	if (!got_network)
		goto out;

	/* check the file exists */
	ret = g_spawn_sync ("/tmp",
			    (gchar **) argv, NULL,
			    0,
			    NULL, NULL,
			    NULL, NULL,
			    &exit_status,
			    &error);
	if (!ret) {
		g_warning ("Failed to run %s: %s", argv[0], error->message);
		g_error_free (error);
		goto out;
	}

	/* we get rc:0 for success, and rc:8 for not found */
	ret = exit_status == 0;
out:
	return ret;
}

/**
 * appdata_text_fn:
 */
static void
appdata_text_fn (GMarkupParseContext *context,
		 const gchar *text,
		 gsize text_len,
		 gpointer user_data,
		 GError **error)
{
	AppdataHelper *helper = (AppdataHelper *) user_data;
	gchar *temp;
	guint len;
	guint str_len;
	gboolean ret;

	/* ignore translations */
	if (helper->tag_translated)
		return;

	switch (helper->section) {
	case APPDATA_SECTION_ID:
		helper->id = g_strstrip (g_strndup (text, text_len));
		ret = appdata_check_id_for_kind (helper->id, helper->kind);
		if (!ret) {
			appdata_add_problem (helper->problems,
					     APPDATA_PROBLEM_KIND_MARKUP_INVALID,
					     "<id> does not have correct extension for kind");
		}
		break;
	case APPDATA_SECTION_LICENCE:
		if (helper->licence != NULL) {
			g_free (helper->licence);
			appdata_add_problem (helper->problems,
					     APPDATA_PROBLEM_KIND_TAG_DUPLICATED,
					     "<licence> is duplicated");
		}
		helper->licence = g_strstrip (g_strndup (text, text_len));
		if (g_strcmp0 (helper->licence, "CC0") != 0 &&
		    g_strcmp0 (helper->licence, "CC-BY") != 0 &&
		    g_strcmp0 (helper->licence, "CC-BY-SA") != 0)
			appdata_add_problem (helper->problems,
					     APPDATA_PROBLEM_KIND_TAG_INVALID,
					     "<licence> is not valid");
		break;
	case APPDATA_SECTION_URL:
		if (helper->url != NULL) {
			g_free (helper->url);
			appdata_add_problem (helper->problems,
					     APPDATA_PROBLEM_KIND_TAG_DUPLICATED,
					     "<url> is duplicated");
		}
		helper->url = g_strstrip (g_strndup (text, text_len));
		if (!g_str_has_prefix (helper->url, "http://") &&
		    !g_str_has_prefix (helper->url, "https://"))
			appdata_add_problem (helper->problems,
					     APPDATA_PROBLEM_KIND_TAG_INVALID,
					     "<url> does not start with 'http://'");
		break;
	case APPDATA_SECTION_UPDATECONTACT:
		if (helper->updatecontact != NULL) {
			g_free (helper->updatecontact);
			appdata_add_problem (helper->problems,
					     APPDATA_PROBLEM_KIND_TAG_DUPLICATED,
					     "<updatecontact> is duplicated");
		}
		helper->updatecontact = g_strstrip (g_strndup (text, text_len));
		if (g_strcmp0 (helper->updatecontact,
			       "someone_who_cares@upstream_project.org") == 0) {
			appdata_add_problem (helper->problems,
					     APPDATA_PROBLEM_KIND_TAG_INVALID,
					     "<updatecontact> is still set to a dummy value");
		}
		len = g_key_file_get_integer (helper->config,
					      APPDATA_TOOLS_VALIDATE_GROUP_NAME,
					      "LengthUpdatecontactMin", NULL);
		if (strlen (helper->updatecontact) < len) {
			appdata_add_problem (helper->problems,
					     APPDATA_PROBLEM_KIND_STYLE_INCORRECT,
					     "<updatecontact> is too short");
		}
		break;
	case APPDATA_SECTION_PROJECT_GROUP:
		if (helper->project_group != NULL) {
			g_free (helper->project_group);
			appdata_add_problem (helper->problems,
					     APPDATA_PROBLEM_KIND_TAG_DUPLICATED,
					     "<project_group> is duplicated");
		}
		helper->project_group = g_strstrip (g_strndup (text, text_len));
		break;
	case APPDATA_SECTION_NAME:
		if (helper->name != NULL) {
			g_free (helper->name);
			appdata_add_problem (helper->problems,
					     APPDATA_PROBLEM_KIND_TAG_DUPLICATED,
					     "<name> is duplicated");
		}
		helper->name = g_strstrip (g_strndup (text, text_len));
		len = g_key_file_get_integer (helper->config,
					      APPDATA_TOOLS_VALIDATE_GROUP_NAME,
					      "LengthNameMin", NULL);
		str_len = strlen (helper->name);
		if (str_len < len) {
			appdata_add_problem (helper->problems,
					     APPDATA_PROBLEM_KIND_STYLE_INCORRECT,
					     "<name> is too short");
		}
		len = g_key_file_get_integer (helper->config,
					      APPDATA_TOOLS_VALIDATE_GROUP_NAME,
					      "LengthNameMax", NULL);
		if (str_len > len) {
			appdata_add_problem (helper->problems,
					     APPDATA_PROBLEM_KIND_STYLE_INCORRECT,
					     "<name> is too long");
		}
		if (appdata_has_fullstop_ending (helper->name)) {
			appdata_add_problem (helper->problems,
					     APPDATA_PROBLEM_KIND_STYLE_INCORRECT,
					     "<name> cannot end in '.'");
		}
		break;
	case APPDATA_SECTION_SUMMARY:
		if (helper->summary != NULL) {
			g_free (helper->summary);
			appdata_add_problem (helper->problems,
					     APPDATA_PROBLEM_KIND_TAG_DUPLICATED,
					     "<summary> is duplicated");
		}
		helper->summary = g_strstrip (g_strndup (text, text_len));
		len = g_key_file_get_integer (helper->config,
					      APPDATA_TOOLS_VALIDATE_GROUP_NAME,
					      "LengthSummaryMin", NULL);
		str_len = strlen (helper->summary);
		if (str_len < len) {
			appdata_add_problem (helper->problems,
					     APPDATA_PROBLEM_KIND_STYLE_INCORRECT,
					     "<summary> is too short");
		}
		len = g_key_file_get_integer (helper->config,
					      APPDATA_TOOLS_VALIDATE_GROUP_NAME,
					      "LengthSummaryMax", NULL);
		if (str_len > len) {
			appdata_add_problem (helper->problems,
					     APPDATA_PROBLEM_KIND_STYLE_INCORRECT,
					     "<summary> is too long");
		}
		if (appdata_has_fullstop_ending (helper->summary)) {
			appdata_add_problem (helper->problems,
					     APPDATA_PROBLEM_KIND_STYLE_INCORRECT,
					     "<summary> cannot end in '.'");
		}
		break;
	case APPDATA_SECTION_DESCRIPTION_PARA:
		temp = g_strstrip (g_strndup (text, text_len));
		len = g_key_file_get_integer (helper->config,
					      APPDATA_TOOLS_VALIDATE_GROUP_NAME,
					      "LengthParaMin", NULL);
		if (strlen (temp) < len) {
			/* we don't add the problem now, as we allow a short
			 * paragraph as an introduction to a list */
			helper->previous_para_was_short = TRUE;
		}
		len = g_key_file_get_integer (helper->config,
					      APPDATA_TOOLS_VALIDATE_GROUP_NAME,
					      "LengthParaMax", NULL);
		str_len = strlen (temp);
		if (str_len > len) {
			appdata_add_problem (helper->problems,
					     APPDATA_PROBLEM_KIND_STYLE_INCORRECT,
					     "<p> is too long");
		}
		if (g_str_has_prefix (temp, "This application")) {
			appdata_add_problem (helper->problems,
					     APPDATA_PROBLEM_KIND_STYLE_INCORRECT,
					     "<p> should not start with 'This application'");
		}
		if (temp[str_len - 1] != '.' &&
		    temp[str_len - 1] != '!' &&
		    temp[str_len - 1] != ':') {
			appdata_add_problem (helper->problems,
					     APPDATA_PROBLEM_KIND_STYLE_INCORRECT,
					     "<p> does not end in '.|:|!'");
		}
		g_free (temp);
		helper->number_paragraphs++;
		break;
	case APPDATA_SECTION_DESCRIPTION_UL_LI:
		temp = g_strstrip (g_strndup (text, text_len));
		len = g_key_file_get_integer (helper->config,
					      APPDATA_TOOLS_VALIDATE_GROUP_NAME,
					      "LengthListItemMin", NULL);
		str_len = strlen (temp);
		if (str_len < len) {
			appdata_add_problem (helper->problems,
					     APPDATA_PROBLEM_KIND_STYLE_INCORRECT,
					     "<li> is too short");
		}
		len = g_key_file_get_integer (helper->config,
					      APPDATA_TOOLS_VALIDATE_GROUP_NAME,
					      "LengthListItemMax", NULL);
		if (str_len > len) {
			appdata_add_problem (helper->problems,
					     APPDATA_PROBLEM_KIND_STYLE_INCORRECT,
					     "<li> is too long");
		}
		if (appdata_has_fullstop_ending (temp)) {
			appdata_add_problem (helper->problems,
					     APPDATA_PROBLEM_KIND_STYLE_INCORRECT,
					     "<li> cannot end in '.'");
		}
		g_free (temp);
		break;
	case APPDATA_SECTION_SCREENSHOT:
		temp = g_strstrip (g_strndup (text, text_len));
		if (strlen (temp) == 0) {
			appdata_add_problem (helper->problems,
					     APPDATA_PROBLEM_KIND_VALUE_MISSING,
					     "<screenshot> has no content");
		}
		ret = appdata_screenshot_already_exists (helper, temp);
		if (ret) {
			appdata_add_problem (helper->problems,
					     APPDATA_PROBLEM_KIND_DUPLICATE_DATA,
					     "<screenshot> has duplicated data");
		} else {
			ret = appdata_screenshot_check_remote_url (helper, temp);
			if (!ret) {
				appdata_add_problem (helper->problems,
						     APPDATA_PROBLEM_KIND_URL_NOT_FOUND,
						     "<screenshot> url not found");
			} else {
				g_ptr_array_add (helper->screenshots,
						 g_strdup (temp));
			}
		}
		g_free (temp);
		break;
	default:
		/* ignore */
		break;
	}
}

/**
 * appdata_check_file_for_problems:
 */
GList *
appdata_check_file_for_problems (GKeyFile *config,
				 const gchar *filename)
{
	AppdataHelper *helper = NULL;
	gboolean ret;
	gchar *data;
	GError *error = NULL;
	GList *problems = NULL;
	GMarkupParseContext *context = NULL;
	gsize data_len;
	guint len;
	const GMarkupParser parser = {
		appdata_start_element_fn,
		appdata_end_element_fn,
		appdata_text_fn,
		NULL,
		NULL };

	g_return_val_if_fail (filename != NULL, NULL);

	/* open file */
	ret = g_file_get_contents (filename, &data, &data_len, &error);
	if (!ret) {
		appdata_add_problem (&problems,
				     APPDATA_PROBLEM_KIND_FAILED_TO_OPEN,
				     error->message);
		g_error_free (error);
		goto out;
	}

	/* check file has the correct ending */
	if (!g_str_has_suffix (filename, ".appdata.xml")) {
		appdata_add_problem (&problems,
				     APPDATA_PROBLEM_KIND_FILENAME_INVALID,
				     "incorrect extension, expected '.appdata.xml'");
	}

	/* parse */
	helper = g_new0 (AppdataHelper, 1);
	helper->problems = &problems;
	helper->section = APPDATA_SECTION_UNKNOWN;
	helper->config = config;
	helper->screenshots = g_ptr_array_new_with_free_func (g_free);
	context = g_markup_parse_context_new (&parser, 0, helper, NULL);
	ret = g_markup_parse_context_parse (context, data, data_len, &error);
	if (!ret) {
		appdata_add_problem (&problems,
				     APPDATA_PROBLEM_KIND_MARKUP_INVALID,
				     error->message);
		g_error_free (error);
		goto out;
	}

	/* check for things that have to exist */
	if (helper->id == NULL) {
		appdata_add_problem (&problems,
				     APPDATA_PROBLEM_KIND_TAG_MISSING,
				     "<id> is not present");
	}
	ret = g_key_file_get_boolean (config, APPDATA_TOOLS_VALIDATE_GROUP_NAME,
				      "RequireContactdetails", NULL);
	if (ret && helper->updatecontact == NULL) {
		appdata_add_problem (&problems,
				     APPDATA_PROBLEM_KIND_TAG_MISSING,
				     "<updatecontact> is not present");
	}
	ret = g_key_file_get_boolean (config, APPDATA_TOOLS_VALIDATE_GROUP_NAME,
				      "RequireUrl", NULL);
	if (ret && helper->url == NULL) {
		appdata_add_problem (&problems,
				     APPDATA_PROBLEM_KIND_TAG_MISSING,
				     "<url> is not present");
	}
	if (helper->licence == NULL) {
		appdata_add_problem (&problems,
				     APPDATA_PROBLEM_KIND_TAG_MISSING,
				     "<licence> is not present");
	}
	len = g_key_file_get_integer (helper->config,
				      APPDATA_TOOLS_VALIDATE_GROUP_NAME,
				      "NumberParaMin", NULL);
	if (helper->number_paragraphs < len) {
		appdata_add_problem (&problems,
				     APPDATA_PROBLEM_KIND_STYLE_INCORRECT,
				     "Not enough <p> tags for a good description");
	}
	len = g_key_file_get_integer (helper->config,
				      APPDATA_TOOLS_VALIDATE_GROUP_NAME,
				      "NumberParaMax", NULL);
	if (helper->number_paragraphs > len) {
		appdata_add_problem (&problems,
				     APPDATA_PROBLEM_KIND_STYLE_INCORRECT,
				     "Too many <p> tags for a good description");
	}
	len = g_key_file_get_integer (helper->config,
				      APPDATA_TOOLS_VALIDATE_GROUP_NAME,
				      "NumberScreenshotsMin", NULL);
	if (helper->screenshots->len < len) {
		appdata_add_problem (&problems,
				     APPDATA_PROBLEM_KIND_STYLE_INCORRECT,
				     "Not enough <screenshot> tags");
	}
	len = g_key_file_get_integer (helper->config,
				      APPDATA_TOOLS_VALIDATE_GROUP_NAME,
				      "NumberScreenshotsMax", NULL);
	if (helper->screenshots->len > len) {
		appdata_add_problem (&problems,
				     APPDATA_PROBLEM_KIND_STYLE_INCORRECT,
				     "Too many <screenshot> tags");
	}
	if (helper->screenshots->len > 0 && !helper->has_default_screenshot) {
		appdata_add_problem (helper->problems,
				     APPDATA_PROBLEM_KIND_MARKUP_INVALID,
				     "<screenshots> has no default <screenshot>");
	}
	if (helper->summary != NULL && helper->name != NULL &&
	    strlen (helper->summary) < strlen (helper->name)) {
		appdata_add_problem (&problems,
				     APPDATA_PROBLEM_KIND_STYLE_INCORRECT,
				     "<summary> is shorter than <name>");
	}
	ret = g_key_file_get_boolean (config, APPDATA_TOOLS_VALIDATE_GROUP_NAME,
				      "RequireTranslations", NULL);
	if (ret) {
		if (helper->name != NULL &&
		    helper->translations_name == 0) {
			appdata_add_problem (&problems,
					     APPDATA_PROBLEM_KIND_TRANSLATIONS_REQUIRED,
					     "<name> has no translations");
		}
		if (helper->summary != NULL &&
		    helper->translations_summary == 0) {
			appdata_add_problem (&problems,
					     APPDATA_PROBLEM_KIND_TRANSLATIONS_REQUIRED,
					     "<summary> has no translations");
		}
		if (helper->number_paragraphs > 0 &&
		    helper->translations_description == 0) {
			appdata_add_problem (&problems,
					     APPDATA_PROBLEM_KIND_TRANSLATIONS_REQUIRED,
					     "<description> has no translations");
		}
	}
out:
	if (helper != NULL) {
		g_free (helper->id);
		g_free (helper->licence);
		g_free (helper->url);
		g_free (helper->name);
		g_free (helper->summary);
		g_free (helper->updatecontact);
		g_free (helper->project_group);
		g_ptr_array_unref (helper->screenshots);
	}
	g_free (helper);
	g_free (data);
	if (context != NULL)
		g_markup_parse_context_unref (context);
	return problems;
}
