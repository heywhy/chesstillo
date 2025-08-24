defmodule Nn.MixProject do
  use Mix.Project

  def project do
    [
      app: :nn,
      version: "0.1.0",
      elixir: "~> 1.18",
      start_permanent: Mix.env() == :prod,
      deps: deps()
    ]
  end

  # Run "mix help compile.app" to learn about applications.
  def application do
    [
      extra_applications: [:logger]
    ]
  end

  # Run "mix help deps" to learn about dependencies.
  defp deps do
    [
      # {:axon, "~> 0.7"},
      {:axon, github: "elixir-nx/axon", override: true},
      {:csv, "~> 3.2"},
      {:emlx, github: "elixir-nx/emlx"},
      {:exla, "~> 0.10"},
      {:explorer, "~> 0.11"},
      {:nx, "~> 0.10", override: true},
      {:polaris, "~> 0.1"},
      {:rein, "~> 0.1"},
      {:table_rex, "~> 4.1", override: true}
    ]
  end
end
