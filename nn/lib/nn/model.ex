defmodule NN.Model do
  alias Axon.Loop
  alias Axon.Losses
  alias Axon.ModelState
  alias Nx.Tensor
  alias Polaris.Optimizers

  import Nx.Defn

  require Explorer.DataFrame, as: DF
  require Explorer.Series, as: S

  @enforce_keys [:out]
  defstruct [:out, columns: [], categories: %{}]

  @type t :: %__MODULE__{
          out: Axon.t(),
          categories: map(),
          columns: [{String.t(), S.dtype()}]
        }

  @max_mate_sq 35
  @columns ~w[turn w_king w_rook b_king]

  @spec max_mate_sq() :: pos_integer()
  def max_mate_sq, do: @max_mate_sq

  @spec new(DF.t()) :: t()
  def new(%DF{} = df) do
    columns = Enum.map(@columns, &{&1, S.dtype(df[&1])})
    model = init_model(columns, :v1)

    %__MODULE__{columns: columns, out: model}
  end

  defp init_model(columns, :v1) do
    columns
    |> Enum.map(fn
      {"turn" = col, _dtype} -> Axon.input(col, shape: {nil, 1})
      {col, _dtype} -> Axon.input(col, shape: {nil, 64})
    end)
    |> Axon.concatenate()
    |> Axon.dense(512, activation: :relu)
    |> Axon.dense(32, activation: :relu)
    |> Axon.dense(32, activation: :relu)
    |> Axon.dense(1)
  end

  @spec batch_to_map(Tensor.t(), t()) :: map()
  def batch_to_map(%Tensor{} = tensor, %__MODULE__{columns: columns}) do
    columns
    |> Enum.map(&elem(&1, 0))
    |> Enum.with_index()
    |> then(&do_batch_to_map(tensor, columns: &1))
  end

  @spec train(t(), Enum.t(), Enum.t(), struct()) :: struct()
  def train(
        %__MODULE__{out: model},
        training_data,
        validation_data,
        initial_state \\ ModelState.empty()
      ) do
    model
    |> Loop.trainer(:mean_squared_error, Optimizers.adam())
    |> Loop.metric(:mean_absolute_error, "mae")
    |> Loop.metric(&r2_score/2, "r2score")
    |> Loop.metric(&baseline/2, "baseline")
    |> Loop.validate(model, validation_data)
    |> Loop.run(training_data, initial_state, epochs: 100)
  end

  @spec test(t(), struct(), Enum.t()) :: struct()
  def test(%__MODULE__{out: model}, state, test_data) do
    model
    |> Loop.evaluator()
    |> Loop.metric(:mean_absolute_error, "mae")
    |> Loop.metric(&r2_score/2, "r2score")
    |> Loop.metric(&baseline/2, "baseline")
    |> Loop.run(test_data, state)
  end

  @spec save!(t(), struct()) :: :ok | no_return()
  def save!(%__MODULE__{columns: columns, categories: categories}, state) do
    model_dir!()
    |> Path.join("model.axon")
    |> File.write!(Nx.serialize(state))

    model_dir!()
    |> Path.join("params")
    |> File.write!(:erlang.term_to_binary({columns, categories}))
  end

  @spec predict(map(), atom()) :: number()
  def predict(params, version \\ :v1) do
    {state, columns, categories} = load_model!()
    model = init_model(columns, version)

    arg = transform(params, columns, categories)

    model
    |> Axon.predict(state, arg)
    |> Nx.reshape({})
    |> Nx.to_number()
  end

  defn r2_score(y_pred, y_true) do
    ss_res = Nx.sum(Nx.pow(y_true - y_pred, 2))
    ss_tot = Nx.sum(Nx.pow(y_true - Nx.mean(y_true), 2))
    1 - ss_res / ss_tot
  end

  defnp baseline(y_true, y_pred) do
    y_baseline = Nx.broadcast(Nx.mean(y_true), y_true.shape)
    mse_fn = &Losses.mean_squared_error(&1, &2, reduction: :mean)

    100 * (mse_fn.(y_baseline, y_true) - mse_fn.(y_pred, y_true)) / mse_fn.(y_baseline, y_true)
  end

  defnp do_batch_to_map(batch, opts) do
    opts = keyword!(opts, columns: [])

    inputs_to_map(batch, opts[:columns])
  end

  deftransformp inputs_to_map(batch, columns) do
    n_rows = Nx.shape(batch) |> elem(0)
    bitboard = Enum.to_list(0..63) |> Nx.tensor()

    Enum.reduce(columns, %{}, fn
      {"turn" = column, i}, acc ->
        tensor = Nx.slice(batch, [0, i], [n_rows, 1])

        Map.put(acc, column, tensor)

      {column, i}, acc ->
        tensor = Nx.slice(batch, [0, i], [n_rows, 1])
        tensor = Nx.equal(tensor, bitboard)

        Map.put(acc, column, tensor)
    end)
  end

  defp load_model! do
    state =
      model_dir!()
      |> Path.join("model.axon")
      |> File.read!()
      |> Nx.deserialize()

    {columns, categories} =
      model_dir!()
      |> Path.join("params")
      |> File.read!()
      |> :erlang.binary_to_term()

    {state, columns, categories}
  end

  defp transform(params, columns, _categories) do
    columns = Enum.map(columns, &elem(&1, 0))
    bitboard = Enum.to_list(0..63) |> Nx.tensor() |> Nx.backend_transfer(EMLX.Backend)

    params
    |> Enum.filter(fn {key, _value} -> key in columns end)
    |> Enum.map(fn
      {"turn" = key, value} -> {key, Nx.tensor([[value]])}
      {key, value} -> {key, Nx.tensor([[value]]) |> Nx.equal(bitboard)}
    end)
    |> Enum.into(%{})
  end

  defp model_dir! do
    :nn
    |> Application.app_dir(["priv", "model"])
    |> tap(&File.mkdir_p!(&1))
  end
end
