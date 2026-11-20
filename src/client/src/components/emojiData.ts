// Fluent Emoji（Microsoft，MIT）3D 表情资源，文件位于 public/emoji/<slug>.png

export interface EmojiItem {
  slug: string;
  char: string;
  label: string;
}

export const EMOJIS: EmojiItem[] = [
  { slug: "grinning_face", char: "😀", label: "咧嘴笑" },
  { slug: "grinning_squinting_face", char: "😆", label: "眯眼大笑" },
  { slug: "grinning_face_with_big_eyes", char: "😃", label: "大眼睛笑" },
  { slug: "grinning_face_with_sweat", char: "😅", label: "苦笑流汗" },
  { slug: "rolling_on_the_floor_laughing", char: "🤣", label: "笑到打滚" },
  { slug: "face_with_tears_of_joy", char: "😂", label: "笑哭" },
  { slug: "slightly_smiling_face", char: "🙂", label: "微笑" },
  { slug: "upside-down_face", char: "🙃", label: "倒脸" },
  { slug: "winking_face", char: "😉", label: "眨眼" },
  { slug: "smiling_face_with_halo", char: "😇", label: "天使笑" },
  { slug: "smiling_face_with_hearts", char: "🥰", label: "爱心脸" },
  { slug: "smiling_face_with_heart-eyes", char: "😍", label: "花痴笑" },
  { slug: "star-struck", char: "🤩", label: "星星眼" },
  { slug: "face_blowing_a_kiss", char: "😘", label: "飞吻" },
  { slug: "face_savoring_food", char: "😋", label: "好吃" },
  { slug: "face_with_tongue", char: "😛", label: "吐舌" },
  { slug: "winking_face_with_tongue", char: "😜", label: "眨眼吐舌" },
  { slug: "zany_face", char: "🤪", label: "搞怪" },
  { slug: "squinting_face_with_tongue", char: "😝", label: "眯眼吐舌" },
  { slug: "money-mouth_face", char: "🤑", label: "财迷" },
  { slug: "hugging_face", char: "🤗", label: "拥抱" },
  { slug: "face_with_hand_over_mouth", char: "🤭", label: "捂嘴笑" },
  { slug: "shushing_face", char: "🤫", label: "嘘" },
  { slug: "thinking_face", char: "🤔", label: "思考" },
  { slug: "zipper-mouth_face", char: "🤐", label: "拉链嘴" },
  { slug: "face_with_raised_eyebrow", char: "🤨", label: "挑眉" },
  { slug: "neutral_face", char: "😐", label: "无语" },
  { slug: "expressionless_face", char: "😑", label: "面无表情" },
  { slug: "face_without_mouth", char: "😶", label: "无嘴" },
  { slug: "smirking_face", char: "😏", label: "坏笑" },
  { slug: "unamused_face", char: "😒", label: "不屑" },
  { slug: "face_with_rolling_eyes", char: "🙄", label: "翻白眼" },
  { slug: "grimacing_face", char: "😬", label: "龇牙" },
  { slug: "relieved_face", char: "😌", label: "放松" },
  { slug: "pensive_face", char: "😔", label: "沉思" },
  { slug: "sleepy_face", char: "😪", label: "困" },
  { slug: "drooling_face", char: "🤤", label: "流口水" },
  { slug: "sleeping_face", char: "😴", label: "睡觉" },
  { slug: "face_with_medical_mask", char: "😷", label: "口罩" },
  { slug: "face_with_thermometer", char: "🤒", label: "生病" },
  { slug: "nauseated_face", char: "🤢", label: "恶心" },
  { slug: "sneezing_face", char: "🤧", label: "打喷嚏" },
  { slug: "hot_face", char: "🥵", label: "热" },
  { slug: "cold_face", char: "🥶", label: "冷" },
  { slug: "woozy_face", char: "🥴", label: "醉" },
  { slug: "dizzy", char: "💫", label: "眩晕" },
  { slug: "exploding_head", char: "🤯", label: "爆炸头" },
  { slug: "cowboy_hat_face", char: "🤠", label: "牛仔" },
  { slug: "partying_face", char: "🥳", label: "派对" },
  { slug: "smiling_face_with_sunglasses", char: "😎", label: "墨镜" },
  { slug: "nerd_face", char: "🤓", label: "书呆子" },
  { slug: "face_with_monocle", char: "🧐", label: "放大镜脸" },
  { slug: "confused_face", char: "😕", label: "困惑" },
  { slug: "worried_face", char: "😟", label: "担心" },
  { slug: "slightly_frowning_face", char: "🙁", label: "微皱眉" },
  { slug: "hushed_face", char: "😯", label: "惊讶" },
  { slug: "astonished_face", char: "😲", label: "震惊" },
  { slug: "flushed_face", char: "😳", label: "脸红" },
  { slug: "pleading_face", char: "🥺", label: "委屈" },
  { slug: "fearful_face", char: "😨", label: "害怕" },
  { slug: "anxious_face_with_sweat", char: "😰", label: "冷汗" },
  { slug: "crying_face", char: "😢", label: "流泪" },
  { slug: "loudly_crying_face", char: "😭", label: "大哭" },
  { slug: "face_screaming_in_fear", char: "😱", label: "尖叫" },
  { slug: "confounded_face", char: "😖", label: "苦恼" },
  { slug: "persevering_face", char: "😣", label: "坚持" },
  { slug: "disappointed_face", char: "😞", label: "失望" },
  { slug: "weary_face", char: "😩", label: "疲惫" },
  { slug: "tired_face", char: "😫", label: "累" },
  { slug: "yawning_face", char: "🥱", label: "打哈欠" },
  { slug: "pouting_face", char: "😡", label: "生气" },
  { slug: "angry_face", char: "😠", label: "愤怒" },
  { slug: "smiling_face_with_horns", char: "😈", label: "恶魔笑" },
  { slug: "skull", char: "💀", label: "骷髅" },
  { slug: "clown_face", char: "🤡", label: "小丑" },
  { slug: "ghost", char: "👻", label: "幽灵" },
  { slug: "alien", char: "👽", label: "外星人" },
  { slug: "robot", char: "🤖", label: "机器人" },
  { slug: "pile_of_poo", char: "💩", label: "便便" },
  { slug: "cat_face", char: "🐱", label: "猫" },
  { slug: "dog_face", char: "🐶", label: "狗" },
  { slug: "panda", char: "🐼", label: "熊猫" },
  { slug: "fox", char: "🦊", label: "狐狸" },
  { slug: "bear", char: "🐻", label: "熊" },
  { slug: "lion", char: "🦁", label: "狮子" },
  { slug: "frog", char: "🐸", label: "青蛙" },
  { slug: "monkey_face", char: "🐵", label: "猴子" },
  { slug: "see-no-evil_monkey", char: "🙈", label: "捂眼" },
  { slug: "red_heart", char: "❤️", label: "红心" },
  { slug: "orange_heart", char: "🧡", label: "橙心" },
  { slug: "yellow_heart", char: "💛", label: "黄心" },
  { slug: "green_heart", char: "💚", label: "绿心" },
  { slug: "blue_heart", char: "💙", label: "蓝心" },
  { slug: "purple_heart", char: "💜", label: "紫心" },
  { slug: "black_heart", char: "🖤", label: "黑心" },
  { slug: "broken_heart", char: "💔", label: "心碎" },
  { slug: "sparkling_heart", char: "💖", label: "闪亮心" },
  { slug: "two_hearts", char: "💕", label: "两颗心" },
  { slug: "hundred_points", char: "💯", label: "满分" },
  { slug: "fire", char: "🔥", label: "火" },
  { slug: "party_popper", char: "🎉", label: "派对礼花" },
  { slug: "birthday_cake", char: "🎂", label: "生日蛋糕" },
  { slug: "beer_mug", char: "🍺", label: "啤酒" },
  { slug: "bubble_tea", char: "🧋", label: "奶茶" },
  { slug: "pizza", char: "🍕", label: "披萨" },
  { slug: "hamburger", char: "🍔", label: "汉堡" },
  { slug: "french_fries", char: "🍟", label: "薯条" },
  { slug: "popcorn", char: "🍿", label: "爆米花" },
  { slug: "doughnut", char: "🍩", label: "甜甜圈" },
  { slug: "rainbow", char: "🌈", label: "彩虹" },
  { slug: "sun", char: "☀️", label: "太阳" },
  { slug: "crescent_moon", char: "🌙", label: "月亮" },
  { slug: "star", char: "⭐", label: "星星" },
  { slug: "sparkles", char: "✨", label: "闪亮" },
  { slug: "rocket", char: "🚀", label: "火箭" },
  { slug: "check_mark_button", char: "✅", label: "对勾" },
  { slug: "warning", char: "⚠️", label: "警告" },
  { slug: "light_bulb", char: "💡", label: "灯泡" },
  { slug: "wrapped_gift", char: "🎁", label: "礼物" },
  { slug: "soccer_ball", char: "⚽", label: "足球" },
  { slug: "video_game", char: "🎮", label: "游戏" },
  { slug: "camera", char: "📷", label: "相机" },
  { slug: "musical_notes", char: "🎶", label: "音符" },
  { slug: "headphone", char: "🎧", label: "耳机" },
  { slug: "open_book", char: "📖", label: "书" },
];

export const EMOJI_RECENT_KEY = "linkchat_recent_emojis";
export const EMOJI_RECENT_MAX = 16;

// 读取最近使用（过滤已失效的 slug）
export function loadRecentEmojis(): string[] {
  try {
    const raw = localStorage.getItem(EMOJI_RECENT_KEY);
    if (!raw) return [];
    const parsed: unknown = JSON.parse(raw);
    if (!Array.isArray(parsed)) return [];
    const known = new Set(EMOJIS.map((item) => item.slug));
    return parsed
      .filter(
        (slug): slug is string => typeof slug === "string" && known.has(slug),
      )
      .slice(0, EMOJI_RECENT_MAX);
  } catch {
    return [];
  }
}

export function saveRecentEmojis(slugs: string[]): void {
  try {
    localStorage.setItem(EMOJI_RECENT_KEY, JSON.stringify(slugs));
  } catch {
    // 存储不可用时忽略，仅影响「最近使用」
  }
}
