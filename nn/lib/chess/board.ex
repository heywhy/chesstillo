defmodule Chess.Board do
  alias Chess.T

  @enforce_keys [:ref]
  defstruct [:ref]

  @type t :: %__MODULE__{ref: reference()}

  @constants NN.constants()

  @spec new() :: t()
  def new do
    %__MODULE__{ref: NN.init_chessboard()}
  end

  defp constants, do: @constants

  @spec terminal?(t()) :: boolean()
  def terminal?(%__MODULE__{} = board) do
    NN.game_over?(board.ref)
  end

  @spec set_turn(t(), T.color()) :: t()
  def set_turn(%__MODULE__{} = board, turn) when is_atom(turn) do
    consts = constants()
    turn = Map.fetch!(consts, turn)

    :ok = NN.set_board_turn(board.ref, turn)

    board
  end

  @spec set_piece_loc(t(), %{color: T.color(), piece: T.piece(), square: non_neg_integer()}) ::
          t()
  def set_piece_loc(%__MODULE__{} = board, %{color: c, piece: p, square: s})
      when is_atom(c) and is_atom(p) and is_integer(s) do
    consts = constants()
    c = Map.fetch!(consts, c)
    p = Map.fetch!(consts, p)

    :ok = NN.set_piece_on_board(board.ref, p, c, s)

    board
  end

  @spec to_fen(t()) :: String.t()
  def to_fen(%__MODULE__{} = board), do: NN.chessboard_to_fen(board.ref)

  @spec probe_dtz(t()) :: integer()
  def probe_dtz(%__MODULE__{} = board) do
    NN.syzygy_probe_dtz(board.ref)
  end
end
