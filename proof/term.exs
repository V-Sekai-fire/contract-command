# Reads what proof/term.c wrote, through the decoder this repository ships, and matches each
# one the way a caller does. This is the check that establishes RFD 0124: whether the bytes are
# the term an Elixir caller matches is a question only Erlang answers.
#
#     ./build/weft_interactor_term /tmp/terms && elixir proof/term.exs /tmp/terms

Code.require_file("elixir/reply.ex", File.cwd!())

dir = System.argv() |> List.first() || "."
failures = :counters.new(1, [])

check = fn name, ok? ->
  IO.puts("#{if ok?, do: "ok  ", else: "FAIL"} #{name}")
  unless ok?, do: :counters.add(failures, 1, 1)
end

read = fn f -> Weft.Reply.decode!(File.read!(Path.join(dir, f))) end

# Naming these atoms is what makes them exist, which is what lets to_existing_atom decode them.
# The set a caller can read is the set it has a clause for.
check.("a bare reason is {:error, :no_engine}", match?({:error, :no_engine}, read.("bare.cbor")))

check.("a reason with numbers keeps them as integers",
  match?({:error, {:res_below_minimum, %{got: 512, minimum: 1280}}}, read.("detail.cbor")))

check.("a success is {:ok, map} with atom keys and a binary",
  match?({:ok, %{layers: 9, ms: 171_260, sidecar: "test.psd"}}, read.("ok.cbor")))

{:ok, value} = read.("ok.cbor")
check.("every key is an atom", Enum.all?(Map.keys(value), &is_atom/1))

# An atom no module names must not be decodable, or the set is not closed.
novel = <<0x82, 0xD8, 39, 0x65, "error", 0xD8, 39, 0x6A, "never_seen">>
refused =
  try do
    Weft.Reply.decode!(novel)
    false
  rescue
    ArgumentError -> true
  end

check.("a reason no caller names is refused, not created", refused)

# Trailing bytes mean the writer and the reader disagree about a length. Absorbing them turns a
# truncated reply into a short one, which is a wrong answer rather than an error.
trailing =
  try do
    Weft.Reply.decode!(File.read!(Path.join(dir, "bare.cbor")) <> <<0x00>>)
    false
  rescue
    ArgumentError -> true
  end

check.("trailing bytes are refused, not ignored", trailing)

n = :counters.get(failures, 1)
IO.puts(if n == 0, do: "term: all checks passed", else: "term: FAILED")
System.halt(if n == 0, do: 0, else: 1)
