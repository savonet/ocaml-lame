(lang dune 2.8)
(version 0.3.5)
(name lame)
(source (github savonet/ocaml-lame))
(license GPL-2.0)
(authors "The Savonet Team <savonet-users@lists.sourceforge.net>")
(maintainers "The Savonet Team <savonet-users@lists.sourceforge.net>")

(generate_opam_files true)
(use_standard_c_and_cxx_flags false)

(package
 (name lame)
 (synopsis "MP3 encoding library")
 (description "Bindings for the lame library which provides functions for encoding mp3 files")
 (depends
  conf-lame
  dune
  dune-configurator)
)
