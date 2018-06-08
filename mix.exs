defmodule Luex.MixProject do
  use Mix.Project

  def project do
    [
      app: :luex,
      version: "0.1.0",
      elixir: "~> 1.6",
      start_permanent: Mix.env() == :prod,
      description: description(),
      package: package(),
      deps: deps(),
      aliases: [format: [&format_c/1, "format"]],
      compilers: [:elixir_make] ++ Mix.compilers(),
      make_error_message: make_error_message(),
      make_clean: ["clean"],
      make_env: make_env()
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
      {:elixir_make, "~> 0.4.1", runtime: false},
      {:ex_doc, "~> 0.18.3", only: :dev}
    ]
  end

  defp description,
    do: """
    """

  defp package,
    do: [
      licenses: ["MIT"],
      maintainers: ["Connor Rigby"],
      links: %{"GitHub" => "https://github.com/connorrigbyy/luex"},
      source_url: "https://github.com/connorrigbyy/luex",
      homepage_url: "https://github.com/connorrigbyy/luex"
    ]

  defp format_c([]) do
    astyle =
      System.find_executable("astyle") ||
        Mix.raise("""
        Could not format C code since astyle is not available.
        """)

    System.cmd(astyle, ["-n", "-r", "c_src/*.c", "c_src/*.h"], into: IO.stream(:stdio, :line))
  end

  defp format_c(_args), do: true

  defp make_error_message do
    """
    Luex failed to compile it's C dependencies. See the above error message
    for what exactly went wrong.
    Make sure you have all of the following installed on your system:
    * GCC
    * GNU Make
    * wget
    """
  end

  defp make_env,
    do: %{
      "MIX_ENV" => to_string(Mix.env()),
      "BUILD_DIR" => Mix.Project.build_path(),
      "DEPS_DIR" => Mix.Project.deps_path(),
      "C_SRC_DIR" => Path.join(__DIR__, "c_src"),
      "PRIV_DIR" => Path.join(__DIR__, "priv"),
      "ERL_EI_INCLUDE_DIR" =>
        System.get_env("ERL_EI_INCLUDE_DIR") || Path.join([:code.root_dir(), "usr", "include"]),
      "ERL_EI_LIBDIR" =>
        System.get_env("ERL_EI_LIBDIR") || Path.join([:code.root_dir(), "usr", "lib"])
    }
end
