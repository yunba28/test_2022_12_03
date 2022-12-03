#include <Siv3D.hpp>

struct SharedData
{
	// ゲームの情報
	struct Game
	{
		// ゲームのタイトル
		String title;

		// ゲーム実行ファイル または URL
		FilePath path;

		// Web ブラウザで起動
		bool isWebApp = false;

		// ゲームの画像
		String textureTag;
		Texture texture()const;

		// ゲームの紹介動画
		String videoTag;
		VideoTexture video()const;

		// 紹介動画の音源
		String audioTag;
		Audio audio()const;

		// ゲームの説明文
		String desc;

		// ゲームの開発スタッフ
		String staff;

		// ゲームの開発ツール
		String tools;

		// マウスを使用するか
		bool useMouse = false;

		// キーボードを使用するか
		bool useKeyboard = false;

		// ゲームパッドを使用するか
		bool useGamepad = false;

		// ランチャー表示優先度（大きいほど優先）
		int32 priority = 0;

	private:

		mutable VideoTexture internalVideoData;

	};

	Array<Game> gameList;
};

bool IsURL(StringView path);

Array<SharedData::Game> LoadGames();

void Main()
{
	auto dataList = LoadGames();

	while (System::Update())
	{
		ClearPrint();

		Print << U"Hello World";
	}
}

bool IsURL(StringView path)
{
	return (path.starts_with(U"http://") || path.starts_with(U"https://"));
}

Array<SharedData::Game> LoadGames()
{
	// ゲームのリスト
	Array<SharedData::Game> games;

	// ホームディレクトリ
	const FilePath homeDirectory = FileSystem::CurrentDirectory() + U"games/";

	// ホームディレクトリにあるアイテムを検索
	for (const FilePath& gameDirectory : FileSystem::DirectoryContents(homeDirectory, Recursive::No))
	{
		// フォルダでない場合はスキップ
		if (not FileSystem::IsDirectory(gameDirectory))
		{
			continue;
		}

		// launcher_info.ini を読み込む
		const FilePath iniPath = (gameDirectory + U"launcher_info.ini");
		const INI ini{ iniPath };

		// 読み込みに失敗
		if (not ini)
		{
			continue;
		}

		// ゲームの情報を読み込む
		SharedData::Game game;
		game.title = ini[U"Game.title"];
		game.textureTag = gameDirectory + ini[U"Game.image"];
		game.videoTag = gameDirectory + ini[U"Game.movie"];
		game.audioTag = gameDirectory + ini[U"Game.movie"];
		game.desc = ini[U"Game.desc"].replaced(U"\\n", U"\n");
		game.staff = ini[U"Game.staff"];
		game.tools = ini[U"Game.tools"];
		game.useMouse = ini.get<bool>(U"Game.mouse");
		game.useKeyboard = ini.get<bool>(U"Game.keyboard");
		game.useGamepad = ini.get<bool>(U"Game.gamepad");
		game.priority = ini.get<int32>(U"Game.priority");

		TextureAsset::Register(game.textureTag, game.textureTag, TextureDesc::Mipped);
		AudioAsset::Register(game.audioTag, Audio::Stream, game.audioTag, Loop::Yes);

		TextureAsset::LoadAsync(game.textureTag);
		AudioAsset::LoadAsync(game.audioTag);

		const String path = game.path = ini[U"Game.path"];
		game.isWebApp = IsURL(path);
		game.path = (game.isWebApp ? path : (gameDirectory + path));

		// ゲームのリストに追加
		games << game;
	}

	// プライオリティに基づいてゲームをソート
	return games.sort_by([](const SharedData::Game& a, const SharedData::Game& b) { return a.priority < b.priority; });
}

Texture SharedData::Game::texture() const
{
	return TextureAsset(textureTag);
}

VideoTexture SharedData::Game::video() const
{
	if (not internalVideoData)
	{
		internalVideoData = VideoTexture{ videoTag,Loop::Yes };
	}
	return internalVideoData;
}

Audio SharedData::Game::audio() const
{
	return AudioAsset(audioTag);
}
