defmodule NN do
  @moduledoc """
  Documentation for `NN`.
  """

  @on_load :load
  @nifs [hello: 0]

  def load do
    path = ~c"#{:code.priv_dir(:nn)}/libnn"

    :ok = :erlang.load_nif(path, 0)
  end

  @doc """
  Hello world.

  ## Examples

      iex> NN.hello()
      :world

  """
  def hello, do: :erlang.nif_error("NIF library not loaded")
end
