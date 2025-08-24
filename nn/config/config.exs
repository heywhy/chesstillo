import Config

config :nx,
  default_defn_options: [compiler: EXLA],
  default_backend: {EXLA.Backend, [client: :host]}

# config :nx,
#   default_backend: {EMLX.Backend, [device: :gpu]},
#   default_defn_options: [compiler: EMLX]
