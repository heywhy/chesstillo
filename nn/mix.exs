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
      {:axon, "~> 0.7"},
      {:exla, "~> 0.10"},
      {:nx, "~> 0.10"},
      {:rein, "~> 0.1"}
    ]
  end
end
