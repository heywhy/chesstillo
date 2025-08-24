defmodule NN do
  @moduledoc """
  Documentation for `NN`.
  """

  @on_load :load

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

  @spec constants :: map()
  def constants, do: :erlang.nif_error("NIF library not loaded")

  @spec init_syzygy_tb(charlist()) :: :ok
  def init_syzygy_tb(_path), do: :erlang.nif_error("NIF library not loaded")

  @spec syzygy_probe_dtz(reference()) :: integer()
  def syzygy_probe_dtz(_ref), do: :erlang.nif_error("NIF library not loaded")

  @spec init_chessboard() :: reference()
  def init_chessboard do
    :erlang.nif_error("NIF library not loaded")
  end

  @spec chessboard_to_fen(reference()) :: String.t()
  def chessboard_to_fen(_ref) do
    :erlang.nif_error("NIF library not loaded")
  end

  @spec set_piece_on_board(reference(), non_neg_integer(), non_neg_integer(), non_neg_integer()) ::
          :ok
  def set_piece_on_board(_board, _piece, _color, _square) do
    :erlang.nif_error("NIF library not loaded")
  end

  @spec set_board_turn(reference(), non_neg_integer()) :: :ok
  def set_board_turn(_board, _turn) do
    :erlang.nif_error("NIF library not loaded")
  end

  @spec game_over?(reference()) :: boolean()
  def game_over?(_board) do
    :erlang.nif_error("NIF library not loaded")
  end
end
