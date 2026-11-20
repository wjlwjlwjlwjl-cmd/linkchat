// 拼音首字母分组（无第三方依赖）
// 借助 zh-Hans 排序规则：汉字按拼音排序，用每个声母的起始汉字做分区边界
const BOUNDARIES: Array<[string, string]> = [
  ["阿", "A"],
  ["八", "B"],
  ["擦", "C"],
  ["搭", "D"],
  ["蛾", "E"],
  ["发", "F"],
  ["噶", "G"],
  ["哈", "H"],
  ["击", "J"],
  ["喀", "K"],
  ["垃", "L"],
  ["妈", "M"],
  ["拿", "N"],
  ["哦", "O"],
  ["啪", "P"],
  ["期", "Q"],
  ["然", "R"],
  ["撒", "S"],
  ["塌", "T"],
  ["挖", "W"],
  ["昔", "X"],
  ["压", "Y"],
  ["匝", "Z"],
];

const collator = new Intl.Collator("zh-Hans-CN", { usage: "sort" });

function initialOfChar(char: string): string {
  if (/[A-Za-z]/.test(char)) return char.toUpperCase();
  if (!/[\u4e00-\u9fff]/.test(char)) return "#";
  let letter = "#";
  for (const [boundary, value] of BOUNDARIES) {
    if (collator.compare(char, boundary) >= 0) {
      letter = value;
    } else {
      break;
    }
  }
  return letter;
}

export interface InitialGroup<T> {
  letter: string;
  items: T[];
}

// 按拼音首字母分组并在组内按拼音排序，组顺序 A-Z，# 放最后
export function groupByInitial<T>(
  items: T[],
  getName: (item: T) => string,
): InitialGroup<T>[] {
  const groups = new Map<string, T[]>();
  for (const item of items) {
    const name = getName(item).trim();
    const letter = name ? initialOfChar(name[0]) : "#";
    const list = groups.get(letter);
    if (list) list.push(item);
    else groups.set(letter, [item]);
  }

  return [...groups.entries()]
    .sort(([a], [b]) => {
      if (a === "#") return 1;
      if (b === "#") return -1;
      return a.localeCompare(b);
    })
    .map(([letter, list]) => ({
      letter,
      items: list.sort((x, y) => collator.compare(getName(x), getName(y))),
    }));
}
