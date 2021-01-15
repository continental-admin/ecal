# -*- coding: utf-8 -*-
#
# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
import os
import sys
# sys.path.insert(0, os.path.abspath('.'))

# -- Generate ecalicons include file-------------------------------------------

import generate_ecalicons
generate_ecalicons.generate_ecalicons("../../app/iconset/ecalicons.qrc", "_include_ecalicons.txt")

# -- Project information -----------------------------------------------------

project = u'eCAL Documentation'
copyright = u'2021, Continental'
author = u'Continental'

# The short X.Y version
version = u''
# The full version, including alpha/beta/rc tags
release = u''


# -- General configuration ---------------------------------------------------

# If your documentation needs a minimal Sphinx version, state it here.
#
# needs_sphinx = '1.0'

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = [
    'sphinx_typo3_theme',
]

# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']

# The suffix(es) of source filenames.
# You can specify multiple suffix as a list of string:
#
# source_suffix = ['.rst', '.md']
source_suffix = '.rst'

# The master toctree document.
master_doc = 'index'

# The language for content autogenerated by Sphinx. Refer to documentation
# for a list of supported languages.
#
# This is also used if you do content translation via gettext catalogs.
# Usually you set "language" from the command line for these cases.
language = None

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = [u'_build', 'Thumbs.db', '.DS_Store']

# The name of the Pygments (syntax highlighting) style to use.
pygments_style = 'sphinx'

# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
html_theme = 'sphinx_typo3_theme'


# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = ['_static']

#https://github.com/TYPO3-Documentation/sphinx_typo3_theme/tree/master/sphinx_typo3_theme
html_theme_options = {
    # Banner
    'logo'                : 'img/ecal-logo.svg',
    'logo_alt'            : 'eCAL Logo',
    'logo_title'          : 'eCAL Documentation',
    'logo_url'            : 'https://continental.github.io/ecal/',

    # Clear deprecated variables to make the "Edit On Github" button work
    'github_branch'       : '',
    'github_commit_hash'  : '',
    'github_repository'   : '',
    'github_revision_msg' : '',
    'github_sphinx_locale': '',

    # Footer
    #'docstypo3org'       : 'True',
    'project_repository' : 'https://github.com/continental/ecal',
}

new_html_context = {
    # "Edit on github" button
    'display_github' : True,
    'github_host'    : 'github.com',
    'github_user'    : 'continental',
    'github_repo'    : 'ecal',
    'github_version' : 'master/',
    'conf_py_path'   : 'doc/rst/',
    'source_suffix'  : '.rst',

    # Misc
    'favicon'        : 'img/favicon.png',
}

if 'html_context' in globals():
    html_context.update(new_html_context)
else:
    html_context = new_html_context

# -- Options for HTMLHelp output ---------------------------------------------

# Output file base name for HTML help builder.
htmlhelp_basename = 'eCALdoc'




# C++ defines used in function definitions
cpp_id_attributes = ['ECAL_API', 'ECALTIME_API']