defmodule Mix.Tasks.Train.Model do
  use Mix.Task

  alias NN.Model

  require Explorer.DataFrame, as: DF
  require Explorer.Series, as: S

  @otp_app :nn

  @requirements "app.start"

  @impl Mix.Task
  def run(argv) do
    {opts, _} = OptionParser.parse!(argv, strict: [batch: :integer])

    batch_size = Keyword.get(opts, :batch, 32)
    path = Application.app_dir(@otp_app, ["priv", "training_data.csv"])

    df = load_dataset!(path)

    {training_df, validation_df, test_df} =
      df
      |> DF.sample(1.0, shuffle: true)
      |> split_dataframe()

    model = Model.new(df)

    test_ds = df_to_dataset(test_df, model, batch_size, false)
    training_ds = df_to_dataset(training_df, model, batch_size)
    validation_ds = df_to_dataset(validation_df, model, batch_size, false)

    inputs =
      model.columns
      |> Enum.map(&elem(&1, 0))
      |> Enum.map(fn
        "turn" = col -> {col, Nx.iota({batch_size, 1}, type: :u8)}
        col -> {col, Nx.iota({batch_size, 64}, type: :u8)}
      end)
      |> Enum.into(%{})

    model.out
    |> Axon.Display.as_table(inputs)
    |> Mix.shell().info()

    Mix.shell().info("training...")

    state = Model.train(model, training_ds, validation_ds)

    Mix.shell().info("\n\ntesting...")

    Model.test(model, state, test_ds)

    Model.save!(model, state)
  end

  defp load_dataset!(path) do
    df =
      path
      |> DF.from_csv!(header: false)
      |> DF.rename(["fen", "eval"])

    DF.mutate_with(df, fn _ ->
      turn = S.transform(df["fen"], &turn_from_fen/1) |> S.cast(:u8)
      location = S.transform(df["fen"], &(pieces_locs(&1) |> Tuple.to_list()))

      [
        turn: turn,
        w_king: S.transform(location, &Enum.at(&1, 0)) |> S.cast(:u8),
        w_rook: S.transform(location, &Enum.at(&1, 1)) |> S.cast(:u8),
        b_king: S.transform(location, &Enum.at(&1, 2)) |> S.cast(:u8)
      ]
    end)
  end

  defp split_dataframe(df) do
    df_count = DF.n_rows(df)
    training_count = floor(0.8 * df_count)
    remaining_count = df_count - training_count

    test_count = floor(0.5 * remaining_count)
    validation_count = floor(0.5 * remaining_count)

    training_df = DF.slice(df, 0, training_count)
    validation_df = DF.slice(df, training_count, validation_count)
    testing_df = DF.slice(df, training_count + validation_count, test_count)

    {training_df, validation_df, testing_df}
  end

  defp df_to_dataset(df, model, batch_size, shuffle \\ true) do
    df =
      case shuffle do
        false -> df
        true -> DF.shuffle(df)
      end

    batched_labels =
      df["eval"]
      |> Nx.stack(axis: -1)
      |> Nx.to_batched(batch_size)

    features = DF.discard(df, ["fen", "eval", "target"])

    batched_features =
      features
      |> Nx.stack(axis: -1)
      |> Nx.to_batched(batch_size)
      |> Stream.map(&Model.batch_to_map(&1, model))

    Stream.zip(batched_features, batched_labels)
  end

  defp pieces_locs(fen) do
    [board, _] = String.split(fen, " ", parts: 2)

    board
    |> String.split("/")
    |> Enum.with_index()
    |> Enum.reduce({0, 0, 0}, fn {row, index}, {wk, wr, bk} ->
      {wk, wr, bk, _, _} =
        row
        |> String.graphemes()
        |> Enum.reduce({wk, wr, bk, 7 - index, 0}, fn char, {wk, wr, bk, row, col} ->
          case char do
            "k" ->
              index = square_index(row, col)

              {wk, wr, index, row, col}

            "K" ->
              index = square_index(row, col)

              {index, wr, bk, row, col}

            "R" ->
              index = square_index(row, col)

              {wk, index, bk, row, col}

            char when char in ~w[1 2 3 4 5 6 7 8] ->
              num = String.to_integer(char)

              {wk, wr, bk, row, col + num}
          end
        end)

      {wk, wr, bk}
    end)
  end

  defp square_index(row, col), do: row * 8 + col

  defp turn_from_fen(fen) do
    [_, turn, _] = String.split(fen, " ", parts: 3)

    case turn do
      "w" -> 0
      "b" -> 1
    end
  end
end
