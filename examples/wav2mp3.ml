(*
 Copyright 2003 Savonet team

 This file is part of OCaml-Vorbis.

 OCaml-Vorbis is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 OCaml-Vorbis is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with OCaml-Vorbis; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *)

(**
  An wav to mp3 converter using OCaml-Lame.

  @author Samuel Mimram
  *)

open Unix


let src = ref ""
let dst = ref ""

            (*
let title = ref None
let artist = ref None
let genre = ref None
let date = ref None
let album = ref None
let tracknum = ref None
             *)
let buflen = ref 1024

let input_string chan len =
  let ans = String.create len in
    (* TODO: check length *)
    ignore (input chan ans 0 len) ;
    (Bytes.unsafe_to_string ans)

let input_int chan =
  let buf = input_string chan 4 in
    (int_of_char buf.[0])
    + (int_of_char buf.[1]) lsl 8
    + (int_of_char buf.[2]) lsl 16
    + (int_of_char buf.[3]) lsl 24

let input_short chan =
  let buf = input_string chan 2 in
    (int_of_char buf.[0]) + (int_of_char buf.[1]) lsl 8

let stereo = ref true
let freq = ref 44100
let bitrate = ref 128
let usage = "usage: wav2mp3 [options] source destination"

let _ =
  Arg.parse
    [ "--sample-freq", Arg.Int (fun f -> freq := f),
      "Sample frequency, default to 44100Hz" ;
      "--mono", Arg.Clear stereo,
      "Do not encode in stereo" ;
      "--bitrate", Arg.Int (fun b -> bitrate := b),
      "Bitrate, in bits per second, defaults to 128kbps" ;
      "--buflen", Arg.Int (fun i -> buflen := i),
      "Size of chunks successively encoded"
    ]
    (
      let pnum = ref (-1) in
        (fun s -> incr pnum; match !pnum with
           | 0 -> src := s
           | 1 -> dst := s
           | _ -> Printf.eprintf "Error: too many arguments\n"; exit 1
        )
    ) usage;
  if !src = "" || !dst = "" then
    (
      Printf.printf "%s\n" usage;
      exit 1
    );
  let ic = open_in_bin !src in
  let oc = open_out_bin !dst in
    if input_string ic 4 <> "RIFF" then invalid_arg "No RIFF tag";
    ignore (input_string ic 4);
    if input_string ic 4 <> "WAVE" then invalid_arg "No WAVE tag";
    if input_string ic 4 <> "fmt " then invalid_arg "No fmt tag";
    let _ = input_int ic in
    let wav_type = input_short ic in (* TODO: should be 1 *)
    let channels = input_short ic in
    let infreq = input_int ic in
    let _ = input_int ic in (* bytes / s *)
    let _ = input_short ic in (* block align *)
    let bits = input_short ic in
    (* Skip to data part *)
    let rec skip () =
      match input_string ic 4 with
      | "data" -> ()
      | "LIST" ->
         let n = input_int ic in
         ignore (input_string ic n);
         skip ()
      | _ -> assert false
    in
    skip ();
    (* let enc_params =
      {
        Vorbis.enc_bitrate = Some !bitrate ;
        Vorbis.enc_min_bitrate = None;
        Vorbis.enc_max_bitrate = None;
        Vorbis.enc_quality = 0.5;
        Vorbis.enc_channels = if !stereo then 2 else 1 ;
        Vorbis.enc_sample_freq = Some !freq;
        Vorbis.enc_managed = false;
        Vorbis.enc_in_channels = channels;
        Vorbis.enc_in_sample_freq = infreq;
        Vorbis.enc_in_sample_size = bits;
        Vorbis.enc_in_big_endian = false;
      } in *)
    let enc = Lame.create_encoder () in
    let start = Unix.time () in
      Lame.init_params enc;
      Printf.printf
        "Input detected: PCM WAVE %d channels, %d Hz, %d bits\n%!"
        channels infreq bits;
      Printf.printf
        "Encoding to: MP3 %d channels, %d Hz, %d kbps\nPlease wait...\n%!"
        (if !stereo then 2 else 1) !freq !bitrate ;
      (* output oc header 0 (String.length header); *)
      let buflen = !buflen in
      let buf = String.create buflen in
        begin try while true do
          really_input ic buf 0 buflen;
          let buf = Bytes.unsafe_to_string buf in
          let outbuf = Lame.encode_buffer enc buf (buflen / 4) in
            output oc (Bytes.of_string outbuf) 0 (String.length outbuf);
        done;
        with End_of_file -> () end;
        let outbuf = Lame.encode_flush enc in
          output oc (Bytes.of_string outbuf) 0 (String.length outbuf);
          Printf.printf "Finished in %.0f seconds.\n" ((Unix.time ())-.start);
          Gc.full_major ()
