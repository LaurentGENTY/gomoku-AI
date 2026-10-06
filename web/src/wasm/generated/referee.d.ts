import type { ModuleOptions, RefereeModule } from '../types';

declare const createReferee: (options?: ModuleOptions) => Promise<RefereeModule>;
export default createReferee;
