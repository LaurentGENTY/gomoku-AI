import type { ModuleOptions, PlayerModule } from '../types';

declare const createPlayer: (options?: ModuleOptions) => Promise<PlayerModule>;
export default createPlayer;
