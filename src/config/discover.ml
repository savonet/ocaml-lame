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

let link_flags = ["-lmp3lame"; "-lm"]

let () =
  C.main ~name:"ocaml-lame" (fun c ->
      C.C_define.gen_header_file c ~fname:"config.h"
        [("BIGENDIAN", Switch (is_big_endian ()))];

      assert (C.c_test ~link_flags c has_lame_h_code);
      C.Flags.write_sexp "c_flags.sexp" [];
      C.Flags.write_sexp "c_library_flags.sexp" link_flags)
