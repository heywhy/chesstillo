defmodule Mix.Tasks.Generate.Training.Data do
  use Mix.Task

  alias Chess.Board

  @otp_app :nn

  @impl Mix.Task
  def run(_) do
    :ok = init_syzygy_tb()
    path = Application.app_dir(@otp_app, ["priv", "training_data.csv"])

    :ok =
      Stream.resource(
        fn -> 0 end,
        fn
          63 = bk_sq ->
            {:halt, bk_sq}

          bk_sq ->
            wk_sqs = gen_random_sqs(64)
            wr_sqs = gen_random_sqs(64)

            pairs = gen_pairs(wk_sqs, wr_sqs, bk_sq)

            {pairs, bk_sq + 1}
        end,
        fn _ -> :ok end
      )
      |> Stream.reject(&is_nil/1)
      |> Stream.map(&Tuple.to_list/1)
      |> CSV.encode()
      |> Stream.into(File.stream!(path))
      |> Stream.run()
  end

  defp gen_pairs(wk_sqs, wr_sqs, bk_sq, acc \\ [])

  defp gen_pairs([], _, _, acc), do: acc

  defp gen_pairs([wk_sq | wk_sqs], wr_sqs, bk_sq, acc) when wk_sq == bk_sq do
    gen_pairs(wk_sqs, wr_sqs, bk_sq, acc)
  end

  defp gen_pairs([wk_sq | wk_sqs], wr_sqs, bk_sq, acc) do
    acc =
      Enum.reduce(wr_sqs, acc, fn wr_sq, acc ->
        case wr_sq == wk_sq or wr_sq == bk_sq do
          true ->
            acc

          false ->
            w_pair = to_pair(bk_sq, wk_sq, wr_sq, :white)
            b_pair = to_pair(bk_sq, wk_sq, wr_sq, :black)

            Enum.concat([w_pair, b_pair], acc)
        end
      end)

    gen_pairs(wk_sqs, wr_sqs, bk_sq, acc)
  end

  defp to_pair(bk_sq, wk_sq, wr_sq, turn) do
    board =
      Board.new()
      |> Board.set_turn(turn)
      |> Board.set_piece_loc(%{color: :black, piece: :king, square: bk_sq})
      |> Board.set_piece_loc(%{color: :white, piece: :king, square: wk_sq})
      |> Board.set_piece_loc(%{color: :white, piece: :rook, square: wr_sq})

    case distance_to_mate(board) do
      {:ok, eval} -> {Board.to_fen(board), eval}
      {:error, :game_over} -> nil
    end
  end

  defp distance_to_mate(board) do
    case Board.terminal?(board) do
      true -> {:error, :game_over}
      false -> {:ok, Board.probe_dtz(board)}
    end
  end

  defp gen_random_sqs(count, acc \\ [])

  defp gen_random_sqs(0, acc), do: acc

  defp gen_random_sqs(count, acc) do
    sq = Enum.random(0..63)
    gen_random_sqs(count - 1, [sq | acc])
  end

  defp init_syzygy_tb do
    path =
      @otp_app
      |> Application.app_dir(["priv", "tb"])
      |> String.to_charlist()

    NN.init_syzygy_tb(path)
  end
end
