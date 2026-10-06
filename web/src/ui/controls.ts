import { BLACK, WHITE, type Color, type Level, type Side } from '../game/types';

export type Mode = 'human' | 'aivsai';

export interface Settings {
  mode: Mode;
  humanColor: Color;
  /** AI level in Human vs AI, Black AI in AI vs AI */
  levelA: Level;
  /** White AI in AI vs AI */
  levelB: Level;
}

const LEVEL_NAMES: readonly Level[] = ['easy', 'medium', 'hard'];

const asLevel = (value: string | null, fallback: Level): Level =>
  LEVEL_NAMES.includes(value as Level) ? (value as Level) : fallback;

export function settingsFromQuery(search: string): Settings {
  const query = new URLSearchParams(search);
  return {
    mode: query.get('mode') === 'aivsai' ? 'aivsai' : 'human',
    humanColor: query.get('color') === 'white' ? WHITE : BLACK,
    levelA: asLevel(query.get('a'), 'medium'),
    levelB: asLevel(query.get('b'), 'hard'),
  };
}

export function sidesFor(settings: Settings): { black: Side; white: Side } {
  if (settings.mode === 'aivsai') {
    return { black: { kind: 'ai', level: settings.levelA }, white: { kind: 'ai', level: settings.levelB } };
  }
  const ai: Side = { kind: 'ai', level: settings.levelA };
  return settings.humanColor === BLACK ? { black: { kind: 'human' }, white: ai } : { black: ai, white: { kind: 'human' } };
}

export interface ControlsView {
  read(): Settings;
  onNewGame(handler: () => void): void;
}

export function bindControls(root: HTMLElement, initial: Settings): ControlsView {
  const mode = root.querySelector<HTMLSelectElement>('#mode')!;
  const color = root.querySelector<HTMLSelectElement>('#human-color')!;
  const levelA = root.querySelector<HTMLSelectElement>('#level-a')!;
  const levelB = root.querySelector<HTMLSelectElement>('#level-b')!;
  const levelALabel = root.querySelector<HTMLElement>('#level-a-label')!;

  mode.value = initial.mode;
  color.value = initial.humanColor === BLACK ? 'black' : 'white';
  levelA.value = initial.levelA;
  levelB.value = initial.levelB;

  const sync = () => {
    const aiVsAi = mode.value === 'aivsai';
    root.querySelectorAll<HTMLElement>('[data-mode="human"]').forEach((el) => (el.hidden = aiVsAi));
    root.querySelectorAll<HTMLElement>('[data-mode="aivsai"]').forEach((el) => (el.hidden = !aiVsAi));
    levelALabel.textContent = aiVsAi ? 'Black AI' : 'AI level';
  };
  mode.addEventListener('change', sync);
  sync();

  return {
    read: () => ({
      mode: mode.value === 'aivsai' ? 'aivsai' : 'human',
      humanColor: color.value === 'white' ? WHITE : BLACK,
      levelA: asLevel(levelA.value, 'medium'),
      levelB: asLevel(levelB.value, 'hard'),
    }),
    onNewGame: (handler) => root.querySelector<HTMLButtonElement>('#new-game')!.addEventListener('click', handler),
  };
}
