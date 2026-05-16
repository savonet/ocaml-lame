module C = Configurator.V1

external is_big_endian : unit -> bool = "ocaml_lame_is_big_endian"

let has_lame_h_code =
  {|
#include <lame/lame.h>

int main()
{
  lame_init();
  return 0;
}
|}

let default_flags = ["-lmp3lame"; "-lm"]

let () =
  C.main ~name:"ocaml-lame" (fun c ->
      C.C_define.gen_header_file c ~fname:"config.h"
        [("BIGENDIAN", Switch (is_big_endian ()))];
      let conf =
        match C.Pkg_config.get c with
        | Some pc -> (
            match C.Pkg_config.query_expr_err pc ~package:"mp3lame" ~expr:"mp3lame" with
            | Ok deps -> deps
            | Error _ -> (
                match C.Pkg_config.query_expr_err pc ~package:"lame" ~expr:"lame" with
                | Ok deps -> deps
                | Error _ -> { C.Pkg_config.libs = default_flags; cflags = [] }))
        | None ->
            assert (C.c_test ~link_flags:default_flags c has_lame_h_code);
            { C.Pkg_config.libs = default_flags; cflags = [] }
      in
      C.Flags.write_sexp "c_flags.sexp" conf.cflags;
      C.Flags.write_sexp "c_library_flags.sexp" conf.libs)
