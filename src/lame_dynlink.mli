module type Lame_t =
  sig
    val get_lame_version : unit -> string
    val get_lame_short_version : unit -> string
    val get_lame_very_short_version : unit -> string
    val get_lame_url : unit -> string
    val get_psy_version : unit -> string
    type encoder
    val create_encoder : unit -> encoder
    val set_in_samplerate : encoder -> int -> unit
    val set_num_channels : encoder -> int -> unit
    val set_out_samplerate : encoder -> int -> unit
    val set_quality : encoder -> int -> unit
    type mode = Stereo | Joint_stereo | Dual_channel | Mono
    val set_mode : encoder -> mode -> unit
    type vbr_mode = Vbr_off | Vbr_rh | Vbr_abr | Vbr_mtrh | Vbr_max_indicator
    val set_vbr_mode : encoder -> vbr_mode -> unit
    val set_vbr_quality : encoder -> int -> unit
    val set_vbr_mean_bitrate : encoder -> int -> unit
    val set_vbr_min_bitrate : encoder -> int -> unit
    val set_vbr_max_bitrate : encoder -> int -> unit
    val set_vbr_hard_min : encoder -> bool -> unit
    val set_brate : encoder -> int -> unit
    val set_compression_ratio : encoder -> float -> unit
    val set_private : encoder -> bool -> unit
    val get_private : encoder -> bool
    val set_copyright : encoder -> bool -> unit
    val get_copyright : encoder -> bool
    val set_original : encoder -> bool -> unit
    val get_original : encoder -> bool
    exception Init_params_failed
    val init_params : encoder -> unit
    val init_bitstream : encoder -> unit
    exception Init_params_not_called
    exception Psychoacoustic_problem
    exception Unknown_error of int
    val encode_buffer_part : encoder -> string -> int -> int -> string
    val encode_buffer : encoder -> string -> int -> string
    val encode_buffer_float_part :
      encoder -> float array -> float array -> int -> int -> string
    val encode_buffer_float :
      encoder -> float array -> float array -> int -> string
    val encode_flush : encoder -> string
    val encode_flush_nogap : encoder -> string
    module Id3tag :
      sig
        val init : encoder -> unit
        val set_title : encoder -> string -> unit
        val set_artist : encoder -> string -> unit
        val set_album : encoder -> string -> unit
        val set_year : encoder -> string -> unit
        val set_comment : encoder -> string -> unit
        val set_track : encoder -> string -> unit
        val set_genre : encoder -> string -> unit
      end
    type mpeg_version = Mpeg_1 | Mpeg_2 | Mpeg_2_5
    val get_version : encoder -> mpeg_version
    val get_encoder_delay : encoder -> int
    val get_framesize : encoder -> int
    val get_nb_samples_to_encode : encoder -> int
    val get_nb_encoded_frames : encoder -> int
    val get_nb_frames : encoder -> int
  end
type handler = { mutable lame_module : (module Lame_t) option; }
val handler : handler
